#ifndef DarknessHandler_h_
#define DarknessHandler_h_

#include <Arduino.h>
#include "WebSocketEventHandler.h"
#include "Led.h"

class DarknessHandler: public Dimmable
{
private:
    WebSocketEventHandler* eventHandler;
    std::vector<Led*> leds;
    bool lightsOn = false;
    
protected:
    bool darknessHandlerEnabled = true;
    unsigned long sunsetTimestamp = 0;
    void switchOff()
    {
        for (auto& led : this->leds) {
            led->switchOff();
        }
    };

public:
    void addLed(Led* led);
    String outputNightTime();

    DarknessHandler(WebSocketEventHandler* eventHandle)
    {
        this->eventHandler = eventHandler;
    };

    virtual void handleDarkness(bool isDark)
    {
        if (!this->darknessHandlerEnabled) {
            return;
        }

        if (isDark && !this->lightsOn) {
            this->lightsOn = true;
            this->sunsetTimestamp = millis();

            for (auto& led : this->leds) {
                led->switchOn();
            }
        }
        else if (!isDark && this->lightsOn) {
            this->lightsOn = false;
            this->eventHandler->textAll(this->outputNightTime());
            this->switchOff();
        }
    };
    
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
    
    if (this->darknessHandlerEnabled) {
        this->lightsOn = false;
        for (auto& led : this->leds) {
            if (led->isOn()) {
                this->lightsOn = true;
                return;
            }
        }
    }
}

void DarknessHandler::addLed(Led* led)
{
    this->leds.push_back(led);

    if (led->isOn()) {
        this->lightsOn = true;
    }
}

String DarknessHandler::outputNightTime()
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
}

#endif  // DarknessHandler_h_