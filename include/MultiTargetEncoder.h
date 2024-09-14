#ifndef MultiTargetEncoder_h_
#define MultiTargetEncoder_h_

#include <Arduino.h>
#include <vector>
#include "TargetSwitcher.h"
#include "Dimmable.h"

class MultiTargetEncoder
{
    private:
        long encoderPosition = -999;
        TargetSwitcher* targetSwitcher;
        std::vector<Dimmable*> dimmableTargets;
        unsigned long lastInputTime = 0;

    public:
        MultiTargetEncoder(TargetSwitcher* targetSwitcher);
        void addDimmable(Dimmable* dimmable);
        unsigned int getTargetLevel();
        long setEncoderPosition(long encoderPosition);
};

MultiTargetEncoder::MultiTargetEncoder(TargetSwitcher* targetSwitcher)
{
    this->targetSwitcher = targetSwitcher;
}

long MultiTargetEncoder::setEncoderPosition(long encoderPosition)
{ 
    const unsigned long timeout = 10000;  // 10 Sekunden Timeout
    
    if (encoderPosition == this->encoderPosition) {
        if (this->lastInputTime == 0) {
            return encoderPosition;
        }

        if (millis() - this->lastInputTime > timeout) {
            this->lastInputTime = 0;
            this->targetSwitcher->reset();
        }

        return encoderPosition;
    }


  const int direction = encoderPosition > this->encoderPosition ? 1 : -1;
  const int target = this->targetSwitcher->getTarget();

  this->dimmableTargets[target]->dim(direction);  // Zeiger dereferenzieren und Methode der abgeleiteten Klasse aufrufen
  this->encoderPosition = encoderPosition;
  this->lastInputTime = millis();

  return encoderPosition;
}

void MultiTargetEncoder::addDimmable(Dimmable* dimmable) 
{
    this->dimmableTargets.push_back(dimmable);
    this->targetSwitcher->setTargetCount(this->dimmableTargets.size() - 1);
}

unsigned int MultiTargetEncoder::getTargetLevel()
{
    return this->dimmableTargets[this->targetSwitcher->getTarget()]->getLevel();
}

#endif  // MultiTargetEncoder_h_