#ifndef RGB_Fire_Wave_h_
#define RGB_Fire_Wave_h_

#include <Arduino.h>
#include "Led.h"

class RGBFireWave: public Led
{    
    public:
        void switchOn() override;
        void switchOff() override;
        bool isOn() override;

        RGBFireWave(): Led(pin) {
            this->switchOff();
        };

        virtual ~RGBFireWave() {};

    void RGBFireWave::switchOn() {}
    void RGBFireWave::switchOff() {}
};

#endif  // RGB_Fire_Wave_h_