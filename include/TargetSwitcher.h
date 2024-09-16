#ifndef TargetSwitcher_h_
#define TargetSwitcher_h_

#include <Arduino.h>
#include "DarknessHandler.h"

class TargetSwitcher
{
private:
    DarknessHandler* darknessHandler;
    int pin;
    unsigned int currentTarget = 0;
    unsigned int targetCount = 0;
    unsigned long lastDebounceTime = 0;    // Zeit des letzten registrierten Tastendrucks
    const unsigned long debounceDelay = 80;  // Entprellzeit
    static TargetSwitcher* instance;  // Statische Instanzreferenz

    static void IRAM_ATTR switchTarget();  // Statische Funktion für ISR

public:
    TargetSwitcher(int pin, DarknessHandler* darknessHandler);
    void handleSwitchTarget();  // Normale Instanzfunktion, die von der ISR aufgerufen wird
    void setTargetCount(unsigned int targetCount);
    unsigned int getTarget();
    void reset();
};

// Initialisiere den statischen Zeiger
TargetSwitcher* TargetSwitcher::instance = nullptr;

TargetSwitcher::TargetSwitcher(int pin, DarknessHandler* darknessHandler)
{
    this->pin = pin;
    this->darknessHandler = darknessHandler;
    pinMode(this->pin, INPUT);

    // Setze den statischen Zeiger auf die aktuelle Instanz
    instance = this;

    attachInterrupt(digitalPinToInterrupt(this->pin), TargetSwitcher::switchTarget, RISING);
}

// Statische ISR-Funktion
void IRAM_ATTR TargetSwitcher::switchTarget()
{
    if (instance != nullptr)
    {
        // Rufe die Instanzmethode auf, um das Ziel zu aktualisieren
        instance->handleSwitchTarget();
    }
}

void TargetSwitcher::handleSwitchTarget()
{
    unsigned long currentTime = millis();

    if (currentTime - lastDebounceTime > debounceDelay)
    {
        this->currentTarget = (this->currentTarget + 1) % (this->targetCount + 1);
        this->darknessHandler->setLevel(0);

        lastDebounceTime = currentTime;
    }
}

void TargetSwitcher::setTargetCount(unsigned int targetCount)
{
    this->targetCount = targetCount;
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
