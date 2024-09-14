#include <Arduino.h>
#include <Encoder.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include "DimmableLed.h"
#include "Dimmable.h"
#include "SunSensor.h"
#include "TargetSwitcher.h"
#include "MultiTargetEncoder.h"
#include "DarknessHandler.h"

#include "html/index.html"

#ifndef APSSID
#define APSSID "Wikinger"
#define APPSK  "sbu421974"
#endif

const char* ap_ssid = APSSID;
const char* ap_pwd = APPSK;

IPAddress myIP;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

DimmableLed redLed(D0, 255);
DimmableLed greenLed(D1, 0);
DimmableLed blueLed(D2, 0);
SunSensor sensor(A0, 20, 7);

Encoder encoder(D4, D3);
TargetSwitcher ledSwitch(D5);
MultiTargetEncoder multiTargetEncoder(&ledSwitch);
DarknessHandler darknessHandler;

void notifyClients()
{
  ws.textAll(String("sadasd"));
}

// put function declarations here:
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

    char *key = strtok((char*)data, ":");
    char *valueStr = strtok(NULL, ":");

    if (key != NULL && valueStr != NULL) {
      int value = atoi(valueStr);
      Serial.printf("Variable %s auf %d gesetzt\n", key, value);
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
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

  WiFi.softAP(ap_ssid, ap_pwd);

  //initWebSocket();

// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Konfiguriere die Captive-Portal-Webseite
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  //server.begin();

  multiTargetEncoder.addDimmable(&sensor);
  multiTargetEncoder.addDimmable(&redLed);
  multiTargetEncoder.addDimmable(&greenLed);
  multiTargetEncoder.addDimmable(&blueLed);
}

void loop()
{
  ws.cleanupClients();

  long lastEncoderPosition = multiTargetEncoder.setEncoderPosition(encoder.read());
  
  darknessHandler.handleDarkness(sensor.read());

  Serial.print("targetLevel: ");
  Serial.print(multiTargetEncoder.getTargetLevel());
  Serial.print(", encoderPosition: ");
  Serial.println(lastEncoderPosition);  
  Serial.print(", Target: ");
  Serial.println(ledSwitch.getTarget());  

  delay(200);
}