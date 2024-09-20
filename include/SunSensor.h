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
    unsigned int sensitivity;
    bool currentState = false;
    bool isDark()
    {
        unsigned int lowerThreshold = this->threshold->getLevel() - this->sensitivity;
        if (this->sensorValue < lowerThreshold) {
            return this->currentState = true;
        }

        unsigned int upperThreshold = this->threshold->getLevel() + this->sensitivity;
        if (this->sensorValue > upperThreshold)
        {
            return this->currentState = false;
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
    }

    unsigned int getLevel() override
    {
        return this->sensorValue;
    }

    void setLevel(unsigned int level) override
    {
        this->sensorValue = map(0, 1023, 0, this->threshold->getUpperLimit(), level);
    }

    bool read()
    {
        this->setLevel(analogRead(this->pin));
        return this->isDark();
    }
};

#endif  // SunSensor_h_