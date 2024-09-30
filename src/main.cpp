#include <Arduino.h>
#include <Encoder.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

#include "DimmableLed.h"
#include "Dimmable.h"
#include "SunSensor.h"
#include "TargetSwitcher.h"
#include "MultiTargetEncoder.h"
#include "LimitedDarknessHandler.h"
#include "WebSocketEventHandler.h"
#include "ArduinoOTAHandler.h"

#include "html/index.html"

#ifndef APSSID
#define APSSID "Wikinger"
#define APPSK  "sbu421974"
#endif

#define LED_STATUS D4
#define LED_RED D1
#define LED_GREEN D2
#define LED_BLUE D3
#define ENCODER_HIGH D5
#define ENCODER_LOW D6
#define ENCODER_SWITCH D7
#define SENSOR_ANALOG A0


const char* ap_ssid = APSSID;
const char* ap_pwd = APPSK;

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WebSocketEventHandler eventHandler(&ws);
IPAddress myIP;

Led statusLed(LED_STATUS);
DimmableLed redLed(LED_RED, "red", 255);
DimmableLed greenLed(LED_GREEN, "green", 0);
DimmableLed blueLed(LED_BLUE, "blue", 0);
Threshold threshold("threshold", 70, 255);
SunSensor sensor(SENSOR_ANALOG, "sensor", &threshold, 7);

Encoder encoder(ENCODER_LOW, ENCODER_HIGH);
LimitedDarknessHandler darknessHandler(&eventHandler, 1000*3600*4);
TargetSwitcher ledSwitch(ENCODER_SWITCH, &darknessHandler);
ArduinoOTAHandler otaHandler("otahandler", &eventHandler, &statusLed);
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
  if (var == "THRESHOLD_LIMIT") return String(threshold.getUpperLimit());
  if (var == "OTA_UPDATE_ENABLED") return String(otaHandler.getLevel());

  return String();
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(32);
  delay(100);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  boolean result = WiFi.softAP(ap_ssid, ap_pwd);
  if (result == true) {
    myIP = WiFi.softAPIP();
    Serial.printf("Wifi AP connection established. IP: %s \n", myIP.toString().c_str());
  }

  dnsServer.start(53, "*", local_IP);

  otaHandler.setup();

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
  eventHandler.addTarget(&otaHandler);

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.begin();
}

void loop()
{
  dnsServer.processNextRequest();
  ws.cleanupClients();
  multiTargetEncoder.setEncoderPosition(encoder.read());

  darknessHandler.handleDarkness(sensor.read());
  ledSwitch.handleSwitchTarget();
  otaHandler.handle();

  if (ws.getClients().length() > 0) {
    eventHandler.textAll(sensor.getName(), sensor.getLevel());
  }

  delay(500);
}