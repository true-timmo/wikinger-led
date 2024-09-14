#ifndef DarknessHandler_h_
#define DarknessHandler_h_

#include <Arduino.h>
#include "Led.h"

class DarknessHandler
{
private:
    unsigned long sunSetTimestamp = 0;
    std::vector<Led*> leds;
    bool lightsOn = false;
    
public:
    DarknessHandler() {};
    void addLed(Led* led);
    void handleDarkness(bool isDark);
    void outputNightTime();
};

void DarknessHandler::addLed(Led* led)
{
    this->leds.push_back(led);
}

void DarknessHandler::handleDarkness(bool isDark)
{
     if (isDark && !this->lightsOn) {
        this->lightsOn = true;
        this->sunSetTimestamp = millis();

         for (auto& led : this->leds) {
            led->switchOn();
        }
    }
    else if (!isDark && this->lightsOn) {
        this->lightsOn = false;
        this->outputNightTime();

        for (auto& led : this->leds) {
            led->switchOff();
        }
    }
}

void DarknessHandler::outputNightTime()
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

  Serial.println(formattedTime);
}

#endif  // DarknessHandler_h_