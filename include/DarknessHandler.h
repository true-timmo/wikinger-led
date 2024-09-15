#ifndef DarknessHandler_h_
#define DarknessHandler_h_

#include <Arduino.h>
#include "WebSocketEventHandler.h"
#include "Led.h"

class DarknessHandler: public Dimmable
{
private:
    WebSocketEventHandler* eventHandler;
    unsigned long sunSetTimestamp = 0;
    std::vector<Led*> leds;
    bool lightsOn = false;
    bool darknessHandlerEnabled = true;
    
public:
    DarknessHandler(WebSocketEventHandler* eventHandler) {
        this->eventHandler = eventHandler;
    };
    void addLed(Led* led);
    void handleDarkness(bool isDark);
    String outputNightTime();

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

void DarknessHandler::handleDarkness(bool isDark)
{
    if (!this->darknessHandlerEnabled) {
        return;
    }

     if (isDark && !this->lightsOn) {
        this->lightsOn = true;
        this->sunSetTimestamp = millis();

        for (auto& led : this->leds) {
            led->switchOn();
        }
    }
    else if (!isDark && this->lightsOn) {
        this->lightsOn = false;
        this->eventHandler->textAll(this->outputNightTime());
        for (auto& led : this->leds) {
            led->switchOff();
        }
    }
}

String DarknessHandler::outputNightTime()
{
  const long lastNightSeconds = (millis() - this->sunSetTimestamp) / 1000;

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