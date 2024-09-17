#ifndef TargetSwitcher_h_
#define TargetSwitcher_h_

#include <Arduino.h>
#include "DarknessHandler.h"

class TargetSwitcher
{
private:
    DarknessHandler* darknessHandler;
    int pin;
    volatile bool buttonPressed = false;
    volatile bool buttonHeld = false;
    unsigned int currentTarget = 0;
    unsigned int targetCount = 0;
    unsigned long lastDebounceTime = 0;    // Zeit des letzten registrierten Tastendrucks
    const unsigned long debounceDelay = 80;  // Entprellzeit
    static TargetSwitcher* instance;  // Statische Instanzreferenz

    static void IRAM_ATTR switchTarget();  // Statische Funktion für ISR

public:
    TargetSwitcher(int pin, DarknessHandler* darknessHandler)
    {
        this->pin = pin;
        this->darknessHandler = darknessHandler;
        pinMode(this->pin, INPUT);

        // Setze den statischen Zeiger auf die aktuelle Instanz
        instance = this;

        attachInterrupt(digitalPinToInterrupt(this->pin), TargetSwitcher::switchTarget, RISING);
    };

    void handleSwitchTarget();  // Normale Instanzfunktion, die von der ISR aufgerufen wird
    void setTargetCount(unsigned int targetCount);
    unsigned int getTarget();
    void reset();
};

// Initialisiere den statischen Zeiger
TargetSwitcher* TargetSwitcher::instance = nullptr;

// Statische ISR-Funktion
void IRAM_ATTR TargetSwitcher::switchTarget()
{
    if (instance != nullptr)
    {
        instance->buttonPressed = true;
    }
}

void TargetSwitcher::handleSwitchTarget()
{
    noInterrupts();
    bool pressed = this->buttonPressed;
    interrupts();

    if (!pressed) {
        return;
    }

    const unsigned long currentTime = millis();
    if (currentTime - this->lastDebounceTime > this->debounceDelay) {
        this->currentTarget = (this->currentTarget + 1) % (this->targetCount + 1);
        this->darknessHandler->setLevel(0);

        this->lastDebounceTime = currentTime;
        this->buttonPressed = false;
    }
}

void TargetSwitcher::setTargetCount(unsigned int targetCount)
{
    this->targetCount = targetCount;
    this->darknessHandler->setLevel(0);
}

unsigned int TargetSwitcher::getTarget()
{
    return this->currentTarget;
}

void TargetSwitcher::reset()
{
    this->currentTarget = 0;
    this->darknessHandler->setLevel(1);
}

#endif  // TargetSwitcher_h_
