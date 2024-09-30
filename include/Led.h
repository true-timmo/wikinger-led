#ifndef Led_h_
#define Led_h_

#include <Arduino.h>

class Led
{    
    protected:
        int pin;
    public:
        Led(int pin) {
            this->pin = pin;
            pinMode(this->pin, OUTPUT);
            this->switchOff();
        };

        virtual void switchOn() {
            digitalWrite(this->pin, LOW);
        };

        virtual void switchOff() {
            digitalWrite(this->pin, HIGH);
        };

        virtual bool isOn()
        {
             return digitalRead(this->pin) == LOW;
        };

        virtual ~Led() {};
};

#endif  // Led_h_