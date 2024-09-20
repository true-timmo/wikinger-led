#include <Arduino.h>
#include <Encoder.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "DimmableLed.h"
#include "Dimmable.h"
#include "SunSensor.h"
#include "TargetSwitcher.h"
#include "MultiTargetEncoder.h"
#include "LimitedDarknessHandler.h"
#include "WebSocketEventHandler.h"

#include "html/index.html"

#ifndef APSSID
#define APSSID "Wikinger"
#define APPSK  "sbu421974"
#endif

const char* ap_ssid = APSSID;
const char* ap_pwd = APPSK;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WebSocketEventHandler eventHandler(&ws);
IPAddress myIP;

DimmableLed redLed(D1, "red", 255);
DimmableLed greenLed(D2, "green", 0);
DimmableLed blueLed(D3, "blue", 0);
Threshold threshold("darkness", 255);
SunSensor sensor(A0, "sensor", &threshold, 7);

Encoder encoder(D5, D4);
LimitedDarknessHandler darknessHandler(&eventHandler, 1000*3600*4);
TargetSwitcher ledSwitch(D6, &darknessHandler);
MultiTargetEncoder multiTargetEncoder(&ledSwitch, &eventHandler);

void initWebSocket() {
  ws.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
  {
    eventHandler.onEvent(server, client, type, arg, data, len);
  });
  server.addHandler(&ws);
}

String processor(const String& var)
{
  if (var == "RED_VALUE") return String(redLed.getLevel());
  if (var == "GREEN_VALUE") return String(greenLed.getLevel());
  if (var == "BLUE_VALUE") return String(blueLed.getLevel());
  if (var == "SENSOR_VALUE") return String(sensor.getLevel());
  if (var == "THRESHOLD_VALUE") return String(threshold.getLevel()); 
  if (var == "DARKNESS_ENABLED") return String(darknessHandler.getLevel());
  if (var == "THRESHOLD_LIMIT") return String(threshold.getUpperLimit());

  return String();
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(32);
  delay(100);

  boolean result = WiFi.softAP(ap_ssid, ap_pwd);
  if (result == true) {
    myIP = WiFi.softAPIP();
    Serial.printf("Wifi AP connection established. IP: %s \n", myIP.toString().c_str());
  }

  multiTargetEncoder.addDimmable(&threshold);
  multiTargetEncoder.addDimmable(&redLed);
  multiTargetEncoder.addDimmable(&greenLed);
  multiTargetEncoder.addDimmable(&blueLed);

  darknessHandler.addLed(&redLed);
  darknessHandler.addLed(&greenLed);
  darknessHandler.addLed(&blueLed);

  eventHandler.addTarget(&redLed);
  eventHandler.addTarget(&greenLed);
  eventHandler.addTarget(&blueLed);
  eventHandler.addTarget(&threshold);
  eventHandler.addTarget(&darknessHandler);
  eventHandler.addTarget(&sensor);

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Konfiguriere die Captive-Portal-Webseite
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
}

void loop()
{
  ws.cleanupClients();
  ledSwitch.handleSwitchTarget();

  multiTargetEncoder.setEncoderPosition(encoder.read());
  darknessHandler.handleDarkness(sensor.read());

  if (ws.getClients().length() > 0) {
    eventHandler.textAll(sensor.getName(), sensor.getLevel());
  }

  //Serial.print("targetLevel: ");
  //Serial.print(multiTargetEncoder.getTargetLevel());
  //Serial.print(", encoderPosition: ");
  //Serial.println(lastEncoderPosition);  
  //Serial.print(", Target: ");
  //Serial.println(ledSwitch.getTarget());  

  delay(500);
}