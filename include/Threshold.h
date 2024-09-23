#ifndef Threshold_h_
#define Threshold_h_

#include <Arduino.h>
#include "Dimmable.h"

class Threshold: public Dimmable 
{
    private:
        const unsigned int dimMultiplier = 5;
        unsigned int upperLimit;
        unsigned int threshold;

    public:
        Threshold(const char* name, unsigned int threshold, unsigned int upperLimit): Dimmable(name)
        {
            this->setPersistable(true);
            this->upperLimit = upperLimit;
            this->threshold = threshold;
        }

        unsigned int getUpperLimit()
        {
            return this->upperLimit;
        }

        void dim(int level)
        {
            level = this->getLevel() + (level * this->dimMultiplier);
            const unsigned int newLevel = level < 0 ? 0 : level;
            this->setLevel(newLevel);
        }

        unsigned int getLevel() override
        {
            return this->threshold;
        }

        void setLevel(unsigned int level) override
        {
            this->threshold = level < this->upperLimit ? level : this->upperLimit;
        }
};

#endif //Threshold_h_h