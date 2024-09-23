#ifndef WebSocketEventHandler_h_
#define WebSocketEventHandler_h_

#include "Target.h"
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

class WebSocketEventHandler
{
    private:
        const unsigned int eepromSize = 2;
        AsyncWebSocket* ws;
        std::vector<Target*> targets;
        void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

    public:
        WebSocketEventHandler(AsyncWebSocket* ws);
        void textAll(const char* target, unsigned int value);
        void textAll(String log);
        void addTarget(Target* target);
        void updateTarget(const char* name, int value);
        void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len);
};

WebSocketEventHandler::WebSocketEventHandler(AsyncWebSocket* ws) {
    this->ws = ws;
}

void WebSocketEventHandler::addTarget(Target* target)
{
    this->targets.push_back(target);

    const int lastIndex = this->targets.size() - 1;
    const unsigned int eepromIndex = this->eepromSize * lastIndex;
    if (target->isPersistable() && EEPROM.read(eepromIndex) == 1) {
             target->setLevel(EEPROM.read(eepromIndex + 1));
    }
}

void WebSocketEventHandler::updateTarget(const char* name, int value)
{
    unsigned int index = 0;
    for (const auto& target : this->targets) {
        if (strcmp(target->getName(), name) == 0)
        {
            if (target->isPersistable()) {
              const unsigned int eepromIndex = this->eepromSize * index;
              EEPROM.write(eepromIndex, 1);
              EEPROM.write(eepromIndex + 1, value);
              EEPROM.commit();
            }

            target->setLevel(value);
            this->textAll(target->getName(), value);
        }
        ++index;
    }
}

void WebSocketEventHandler::textAll(const char* targetName, unsigned int value)
{
    this->ws->textAll(String(targetName) + ":" + String(value));
}

void WebSocketEventHandler::textAll(String log)
{
    this->ws->textAll(log);
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