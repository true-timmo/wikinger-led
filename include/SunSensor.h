#ifndef SunSensor_h_
#define SunSensor_h_

#include <Arduino.h>
#include "Dimmable.h"

class SunSensor: public Dimmable
{
private:
    int pin;
    const int maxThreshold = 255;
    const unsigned int dimMultiplier = 5;
    unsigned int threshold;
    unsigned int sensitivity;
    bool isDark = false;
    
public:
    SunSensor(int pin, const char* name, unsigned int threshold, unsigned int sensitivity): Dimmable(name)
    {
        this->pin = pin;
        this->threshold = threshold;
        this->sensitivity = sensitivity;

        pinMode(this->pin, INPUT);
    }
    
    const int getMaxThreshold() {
        return this->maxThreshold;
    }

    bool setThreshold(int threshold);
    unsigned int getThreshold();
    void dim(int value) override;
    unsigned int getLevel() override;
    void setLevel(unsigned int level) override;
    bool read();
};

bool SunSensor::setThreshold(int threshold)
{
    this->threshold = constrain(threshold, 0, this->maxThreshold);

    return this->read();
}

unsigned int SunSensor::getThreshold()
{
    return this->threshold;
}

bool SunSensor::read()
{
    const int sensorValue = map(0, 1023, 0, this->maxThreshold, analogRead(this->pin));
    const int lowerThreshold = this->threshold - this->sensitivity;
    const int upperThreshold = this->threshold + this->sensitivity;

    this->isDark = (sensorValue < lowerThreshold) ? true : (sensorValue > upperThreshold) ? false : this->isDark;

    return this->isDark;
}

void SunSensor::dim(int level)
{
    level = level * this->dimMultiplier;

    this->setThreshold(this->getThreshold() + level);
}

unsigned int SunSensor::getLevel()
{
    return this->threshold;
}

void SunSensor::setLevel(unsigned int level)
{
    this->setThreshold(level);
}

#endif  // SunSensor_h_