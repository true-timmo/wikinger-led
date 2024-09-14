#ifndef WebSocketEventHandler_h_
#define WebSocketEventHandler_h_

#include "Dimmable.h"
#include <ESPAsyncWebServer.h>

class WebSocketEventHandler
{
    private:
        AsyncWebSocket* ws;
        std::vector<char*> targetNames;
        std::vector<Dimmable*> targets;
        void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);

    public:
        WebSocketEventHandler(AsyncWebSocket* ws);
        void addTarget(char* name, Dimmable* target);
        void updateTarget(char* name, int value);
        void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len);
};

WebSocketEventHandler::WebSocketEventHandler(AsyncWebSocket* ws) {
    this->ws = ws;
}

void WebSocketEventHandler::addTarget(char* name, Dimmable* target)
{
    this->targetNames.push_back(name);
    this->targets.push_back(target);
}

void WebSocketEventHandler::updateTarget(char* target, int value)
{
    unsigned int index = 0;
    for (const auto& name : this->targetNames) {
        if (strcmp(target, name) == 0)
        {
            this->targets[index]->setLevel(value);
            this->ws->textAll(String(target) + ":" + String(value));
        }
        ++index;
    }
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