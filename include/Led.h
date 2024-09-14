#ifndef Led_h_
#define Led_h_

#include <Arduino.h>

class Led
{    
    protected:
        int pin;
        unsigned int level;
    public:
        Led(int pin) {
            this->pin = pin;
            this->level = LOW;
            pinMode(this->pin, OUTPUT);
        };

        virtual void switchOn() {
            digitalWrite(this->pin, HIGH);
        };

        virtual void switchOff() {
            digitalWrite(this->pin, LOW);
        };

        virtual bool isOn() {
             return this->level != LOW;
        };

        virtual ~Led() {};
};

#endif  // Led_h_