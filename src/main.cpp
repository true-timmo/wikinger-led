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
#include "DarknessHandler.h"
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

DimmableLed redLed(D0, 255);
DimmableLed greenLed(D1, 0);
DimmableLed blueLed(D2, 0);
SunSensor sensor(A0, 20, 7);

Encoder encoder(D4, D3);
TargetSwitcher ledSwitch(D5);
MultiTargetEncoder multiTargetEncoder(&ledSwitch);
DarknessHandler darknessHandler;

void initWebSocket() {
  ws.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
  {
    eventHandler.onEvent(server, client, type, arg, data, len);
  });
  server.addHandler(&ws);
}

String processor(const String& var)
{
  return String();
}

void setup()
{
  Serial.begin(9600);
  delay(100);

  boolean result = WiFi.softAP(ap_ssid, ap_pwd);
  if (result == true) {
    myIP = WiFi.softAPIP();
    Serial.printf("Wifi AP connection established. IP: %s \n", myIP.toString().c_str());
  }

  multiTargetEncoder.addDimmable(&sensor);
  multiTargetEncoder.addDimmable(&redLed);
  multiTargetEncoder.addDimmable(&greenLed);
  multiTargetEncoder.addDimmable(&blueLed);

  eventHandler.addTarget("red", &redLed);
  eventHandler.addTarget("green", &greenLed);
  eventHandler.addTarget("blue", &blueLed);

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

  long lastEncoderPosition = multiTargetEncoder.setEncoderPosition(encoder.read());
  
  darknessHandler.handleDarkness(sensor.read());

  //Serial.print("targetLevel: ");
  //Serial.print(multiTargetEncoder.getTargetLevel());
  //Serial.print(", encoderPosition: ");
  //Serial.println(lastEncoderPosition);  
  //Serial.print(", Target: ");
  //Serial.println(ledSwitch.getTarget());  

  delay(200);
}