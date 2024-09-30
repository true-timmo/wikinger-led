#ifndef ArduinoOTAHandler_h_
#define ArduinoOTAHandler_h_

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "WebSocketEventHandler.h"
#include "Target.h"

class ArduinoOTAHandler: public Target
{
    private:
        static ArduinoOTAHandler* instance;  // Statische Instanzreferenz
        Led* statusLed;
        WebSocketEventHandler* eventHandler;

        void wsTextAll(String text)
        {
            this->eventHandler->textAll(text);
        };
            
    protected:
        bool otaHandlerEnabled = false;

    public:
        ArduinoOTAHandler(const char* name, WebSocketEventHandler* eventHandler, Led* statusLed): Target(name)
        {
            this->eventHandler = eventHandler;
            this->statusLed = statusLed;

            // Setze den statischen Zeiger auf die aktuelle Instanz
            instance = this;
        }

        void setup()
        {
            ArduinoOTA.onStart([]()
            {
                instance->wsTextAll("Start OTA update...");
                delay(100);
            });

            ArduinoOTA.onEnd([]()
            {
                instance->wsTextAll("...OTA update done");
            });

            ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
            {
                instance->wsTextAll("uploading ... " + String(progress / (total / 100)) + "%");
                delay(50);
            });

            ArduinoOTA.onError([](ota_error_t error)
            {
                String errorCode = ("Error[" + String(error) + "]: ");
                if (error == OTA_AUTH_ERROR) instance->wsTextAll(errorCode + "Auth Failed");
                else if (error == OTA_BEGIN_ERROR) instance->wsTextAll(errorCode + "Begin Failed");
                else if (error == OTA_CONNECT_ERROR) instance->wsTextAll(errorCode + "Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) instance->wsTextAll(errorCode + "Receive Failed");
                else if (error == OTA_END_ERROR) instance->wsTextAll(errorCode + "End Failed");
            });

        }

        void handle()
        {
            if (!this->otaHandlerEnabled) {
                return;
            }

            ArduinoOTA.handle();
            this->statusLed->switchOn();
            delay(500);
            this->statusLed->switchOff();
            delay(500);
        }

        unsigned int getLevel() override
        {
            return this->otaHandlerEnabled;
        }

        void setLevel(unsigned int level) override
        {
            this->otaHandlerEnabled = level > 0;

            if (this->otaHandlerEnabled) {
                instance->wsTextAll("OTA waiting for connection...");
                return ArduinoOTA.begin();
            }

            instance->wsTextAll(" ");
            ArduinoOTA.end();
        }
};

// Initialisiere den statischen Zeiger
ArduinoOTAHandler* ArduinoOTAHandler::instance = nullptr;

#endif  // ArduinoOTAHandler_h_