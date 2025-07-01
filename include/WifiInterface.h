#ifndef WifiInterface_h_
#define WifiInterface_h_

#include "Target.h"
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>



class WifiInterface
{
    private:
        const unsigned int eepromSize = 2;
        AsyncWebSocket* ws;
        std::vector<Target*> targets;

    public:
        WifiInterface(AsyncWebSocket* ws);
        void initWebSocket();
};

WifiInterface::WifiInterface(AsyncWebSocket* ws) {
    this->ws = ws;
}

void WifiInterface::initWebSocket() {
    ws.onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
      eventHandler.onEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);
  }
}

#endif // WifiInterface_h_