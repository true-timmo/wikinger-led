#ifndef DarknessHandler_h_
#define DarknessHandler_h_

#include <Arduino.h>
#include "WebSocketEventHandler.h"
#include "Dimmable.h"
#include "Led.h"

class DarknessHandler: public Dimmable
{
    private:
        WebSocketEventHandler* eventHandler;
        std::vector<Led*> leds;
        bool lightsOn = false;
        
        void init() {
            if (!this->darknessHandlerEnabled) {
                this->switchOn();
            }

            if (this->darknessHandlerEnabled) {
                this->switchOff();
            }
        };
    
    protected:
        bool darknessHandlerEnabled = true;
        unsigned long sunsetTimestamp = 0;
        
        void switchOff()
        {
            this->lightsOn = false;
            for (auto& led : this->leds) {
                led->switchOff();
            }
        };

        void switchOn()
        {
            this->sunsetTimestamp = millis();
            this->lightsOn = true;
            for (auto& led : this->leds) {
                led->switchOn();
            }
        };    

    public:
        DarknessHandler(WebSocketEventHandler* eventHandler, const char* name = "darkness"): Dimmable(name)
        {
            this->eventHandler = eventHandler;
        };

        String outputNightTime()
        {
            const long lastNightSeconds = (millis() - this->sunsetTimestamp) / 1000;

            int hours = lastNightSeconds / 3600;
            int minutes = (lastNightSeconds % 3600) / 60;
            int seconds = (lastNightSeconds % 60);

            String formattedTime = String(seconds) + " Sekunden";

            if (minutes > 0) {
                formattedTime = String(minutes) + " Minuten, " + formattedTime;
            }

            if (hours > 0) {
                formattedTime = String(hours) + " Stunden, " + formattedTime;
            }

            return formattedTime;
        };

        virtual void handleDarkness(bool isDark)
        {
            if (!this->darknessHandlerEnabled) {
                return;
            }

            if (isDark && !this->lightsOn) {
                this->switchOn();
            }
            else if (!isDark && this->lightsOn) {   
                this->eventHandler->textAll(this->outputNightTime());
                this->switchOff();
            }
        };
        
        void addLed(Led* led);
        void dim(int value) override {};
        unsigned int getLevel() override;
        void setLevel(unsigned int level) override;
};

unsigned int DarknessHandler::getLevel()
{
    return this->darknessHandlerEnabled;
}

void DarknessHandler::setLevel(unsigned int level)
{
    this->darknessHandlerEnabled = level > 0;
    this->init();
}

void DarknessHandler::addLed(Led* led)
{
    this->leds.push_back(led);

    if (led->isOn()) {
        this->lightsOn = true;
    }
}

#endif  // DarknessHandler_h_