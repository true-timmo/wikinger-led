#ifndef SunSensor_h_
#define SunSensor_h_

#include <Arduino.h>
#include "Target.h"
#include "Threshold.h"

class SunSensor: public Target
{
private:
    int pin;
    Threshold* threshold;
    unsigned int sensorValue;
    unsigned int rawValue = 0;
    unsigned int sensitivity;
    bool currentState = false;
    bool isDark()
    {
        unsigned int lowerThreshold = this->threshold->getLevel() < this->sensitivity ? 0 : this->threshold->getLevel() - this->sensitivity;
        if (this->sensorValue <= lowerThreshold) {
            return this->currentState = false;
        }

        unsigned int upperThreshold = this->threshold->getLevel() + this->sensitivity;
        if (this->sensorValue >= upperThreshold)
        {
            return this->currentState = true;
        }

        return this->currentState;
    }
    
public:
    SunSensor(
        int pin,
        const char* name,
        Threshold* threshold,
        unsigned int sensitivity
    ): Target(name) {
        this->pin = pin;
        this->threshold = threshold;
        this->sensitivity = sensitivity;

        pinMode(this->pin, INPUT);
        analogSetPinAttenuation(this->pin, ADC_11db);
    }

    unsigned int getLevel() override
    {
        return this->sensorValue;
    }

    void setLevel(unsigned int level) override
    {
        this->sensorValue = map(level, 0, 4095, 0, this->threshold->getUpperLimit());
    }

    bool read()
    {
        this->rawValue = analogRead(this->pin);
        this->setLevel(this->rawValue);
        return this->isDark();
    }

    // Rohwert des ADC (0-4095) der letzten Messung
    unsigned int getRawValue()
    {
        return this->rawValue;
    }

    // Gibt die letzte Messung samt Schaltschwellen auf der Konsole aus
    void printStatus()
    {
        const unsigned int t = this->threshold->getLevel();
        Serial.printf("[Sensor] roh=%u wert=%u hell<=%u dunkel>=%u -> %s\n",
            this->rawValue, this->sensorValue,
            t < this->sensitivity ? 0 : t - this->sensitivity,
            t + this->sensitivity,
            this->currentState ? "dunkel" : "hell");
    }
};

#endif  // SunSensor_h_