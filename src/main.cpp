#include <Arduino.h>
#include <Encoder.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <map>

#include "DimmableLed.h"
#include "Dimmable.h"
#include "SunSensor.h"
#include "TempSensor.h"
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

#define LED_STATUS 13
#define LED_RED 25
#define LED_GREEN 26
#define LED_BLUE 27
#define ENCODER_HIGH 32
#define ENCODER_LOW 33
#define ENCODER_SWITCH 13
#define SENSOR_TEMP 35
#define SENSOR_ANALOG 34


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
TempSensor temp(SENSOR_TEMP, "temp");

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
  static const std::map<String, std::function<String()>> lookup = {
    {"RED_VALUE", []() { return String(redLed.getLevel()); }},
    {"GREEN_VALUE", []() { return String(greenLed.getLevel()); }},
    {"BLUE_VALUE", []() { return String(blueLed.getLevel()); }},
    {"SENSOR_VALUE", []() { return String(sensor.getLevel()); }},
    {"TEMP_VALUE", []() { return String(temp.getTemperature()); }},
    {"HUMID_VALUE", []() { return String(temp.getHumidity()); }},
    {"THRESHOLD_VALUE", []() { return String(threshold.getLevel()); }},
    {"DARKNESS_ENABLED", []() { return String(darknessHandler.getLevel()); }},
    {"THRESHOLD_LIMIT", []() { return String(threshold.getUpperLimit()); }},
    {"OTA_UPDATE_ENABLED", []() { return String(otaHandler.getLevel()); }}
  };

  auto it = lookup.find(var);
  if (it != lookup.end()) {
      return it->second(); // Call the associated function
  }

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
  eventHandler.addTarget(&temp);
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
    Serial.printf("Current sensor status: %d \n", sensor.getLevel());
    eventHandler.textAll(sensor.getName(), sensor.getLevel());
    eventHandler.textAll(temp.getName(), temp.getTemperature());
  }

  delay(500);
}