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
            pinMode(LED_BUILTIN, OUTPUT);

            ArduinoOTA.onStart([]()
            {
                String type;
                if (ArduinoOTA.getCommand() == U_FLASH) {
                    type = "sketch";
                } else { // U_SPIFFS
                    type = "filesystem";
                }
                instance->wsTextAll("Start OTA update: " + type);
            });

            ArduinoOTA.onEnd([]()
            {
                instance->wsTextAll("End");
                ESP.restart();
            });

            ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
            {
                float currentProgress = progress / (total / 100);
                instance->wsTextAll("Progress: " + String(currentProgress) + "%");
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
            this->statusLed->switchOff();
            delay(1000);
            this->statusLed->switchOn();
            delay(1000);
        }

        unsigned int getLevel() override
        {
            return this->otaHandlerEnabled;
        }

        void setLevel(unsigned int level) override
        {
            this->otaHandlerEnabled = level > 0;

            if (this->otaHandlerEnabled) {
                ArduinoOTA.begin();
            }
        }
};

// Initialisiere den statischen Zeiger
ArduinoOTAHandler* ArduinoOTAHandler::instance = nullptr;

#endif  // ArduinoOTAHandler_h_