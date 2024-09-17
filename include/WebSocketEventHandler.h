#ifndef WebSocketEventHandler_h_
#define WebSocketEventHandler_h_

#include "Dimmable.h"
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

class WebSocketEventHandler
{
    private:
        const unsigned int eepromSize = 2;
        AsyncWebSocket* ws;
        std::vector<Dimmable*> targets;
        void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

    public:
        WebSocketEventHandler(AsyncWebSocket* ws);
        void textAll(String text);
        void addTarget(Dimmable* target);
        void updateTarget(const char* name, int value);
        void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len);
};

WebSocketEventHandler::WebSocketEventHandler(AsyncWebSocket* ws) {
    this->ws = ws;
}

void WebSocketEventHandler::addTarget(Dimmable* target)
{
    this->targets.push_back(target);

    const int lastIndex = this->targets.size() - 1;
    const unsigned int eepromIndex = this->eepromSize * lastIndex;
    if (EEPROM.read(eepromIndex) == 1) {
             target->setLevel(EEPROM.read(eepromIndex + 1));
    }
}

void WebSocketEventHandler::updateTarget(const char* name, int value)
{
    unsigned int index = 0;
    for (const auto& target : this->targets) {
        if (strcmp(target->getName(), name) == 0)
        {
            //Too lazy to implement a class, so let's hack this for now
            const unsigned int eepromIndex = this->eepromSize * index;
            EEPROM.write(eepromIndex, 1);
            EEPROM.write(eepromIndex + 1, value);
            EEPROM.commit();

            target->setLevel(value);
            this->textAll(String(target->getName()) + ":" + String(value));
        }
        ++index;
    }
}

void WebSocketEventHandler::textAll(String text)
{
    this->ws->textAll(text);
}

void WebSocketEventHandler::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type) {
      case WS_EVT_DATA:
        this->handleWebSocketMessage(arg, data, len);
        break;
      case WS_EVT_CONNECT:
      case WS_EVT_DISCONNECT:
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}

void WebSocketEventHandler::handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    char* receivedData = (char*)data;
    char* target = strtok(receivedData, ":");
    char* valueStr = strtok(NULL, ":");

    if (target != NULL && valueStr != NULL) {
      int value = atoi(valueStr); 
      this->updateTarget(target, value);
    }
  }
}

#endif // WebSocketEventHandler_h_