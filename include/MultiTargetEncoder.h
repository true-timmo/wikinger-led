#ifndef MultiTargetEncoder_h_
#define MultiTargetEncoder_h_

#include <Arduino.h>
#include <vector>
#include "TargetSwitcher.h"
#include "WebSocketEventHandler.h"
#include "Dimmable.h"

class MultiTargetEncoder
{
    private:
        long encoderPosition = -999;
        TargetSwitcher* targetSwitcher;
        WebSocketEventHandler* eventHandler;
        std::vector<Dimmable*> dimmableTargets;
        unsigned long lastInputTime = 0;

    public:
        MultiTargetEncoder(TargetSwitcher* targetSwitcher, WebSocketEventHandler* eventHandler)
        {
            this->targetSwitcher = targetSwitcher;
            this->eventHandler = eventHandler;
        };
        void addDimmable(Dimmable* dimmable);
        unsigned int getTargetLevel();
        long setEncoderPosition(long encoderPosition);
};

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
  Dimmable* target = this->dimmableTargets[this->targetSwitcher->getTarget()];

  target->dim(direction);
  this->eventHandler->textAll(String(target->getName()) + ":" + String(target->getLevel()));
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