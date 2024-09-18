#ifndef DimmableLed_h_
#define DimmableLed_h_

#include <Arduino.h>
#include "Dimmable.h"
#include "Led.h"

class DimmableLed: public Dimmable, public Led
{    
public:
    DimmableLed(int pin, const char* name, unsigned int initialLevel = 0) : Dimmable(name), Led(pin) {
        this->setLevel(initialLevel);
    };

    void dim(int offset) override;
    unsigned int getLevel() override;
    void setLevel(unsigned int level);
    void switchOn() override;
    void switchOff() override;
private:
    unsigned int multiplier = 5;
};

void DimmableLed::setLevel(unsigned int level)
{
    // Begrenze den Level auf den Bereich 0-255
    this->level = (level < 255) ? level : 255;
    this->switchOn();  // Schalte die LED mit dem neuen Level ein
}

void DimmableLed::dim(int offset)
{
    const int level = this->level + (offset * this->multiplier);
    this->setLevel(level > 0 ? level : 0);
}

unsigned int DimmableLed::getLevel()
{
    return this->level;
}

void DimmableLed::switchOn()
{
    analogWrite(this->pin, map(this->level, 0, 255, 0, 255));
}

void DimmableLed::switchOff()
{
    analogWrite(this->pin, 0);
}

#endif  // DimmableLed_h_