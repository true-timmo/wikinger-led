#ifndef TempSensor_h_
#define TempSensor_h_

#include <SimpleDHT.h>
#include "Target.h"

class TempSensor: public Target
{
    private:
        float temperature = 0;
        float humidity = 0;

        SimpleDHT11 dht11;
        int err = SimpleDHTErrSuccess;

    public:
        TempSensor(
            int pin,
            const char* name
        ): Target(name) {
            pinMode(pin, INPUT);

            this->dht11 = SimpleDHT11(pin);
        }

        float getTemperature()
        {
            return this->temperature;
        }

        float getHumidity()
        {
            return this->humidity;
        }

        unsigned int getLevel() override
        {
            return this->temperature;
        }

        void setLevel(unsigned int level) override
        {
            this->err = this->dht11.read2(&this->temperature, &this->humidity, NULL);
        }
};

#endif // TempSensor_h_