#ifndef SunSensor_h_
#define SunSensor_h_

#include <Arduino.h>
#include "Dimmable.h"

class SunSensor: public Dimmable
{
private:
    int pin;
    unsigned int threshold;
    unsigned int sensitivity;
    bool isDark = false;
    
public:
    SunSensor(int pin, unsigned int threshold, unsigned int sensitivity);
    bool setThreshold(int threshold);
    unsigned int getThreshold();
    void dim(int value) override;
    unsigned int getLevel() override;
    bool read();
};

SunSensor::SunSensor(int pin, unsigned int threshold = 0, unsigned int sensitivity = 0)
{
    this->pin = pin;
    this->threshold = threshold;
    this->sensitivity = sensitivity;

    pinMode(this->pin, INPUT);
}

bool SunSensor::setThreshold(int threshold)
{
    this->threshold = constrain(threshold, 0, 128);

    return this->read();
}

unsigned int SunSensor::getThreshold()
{
    return this->threshold;
}

bool SunSensor::read()
{
    const int sensorValue = 0; //analogRead(this->pin);
    const int lowerThreshold = this->threshold - this->sensitivity;
    const int upperThreshold = this->threshold + this->sensitivity;

    this->isDark = (sensorValue < lowerThreshold) ? true : (sensorValue > upperThreshold) ? false : this->isDark;

    return this->isDark;
}

void SunSensor::dim(int level)
{
    this->setThreshold(this->getThreshold() + level);
}

unsigned int SunSensor::getLevel()
{
    return this->threshold;
}

#endif  // SunSensor_h_