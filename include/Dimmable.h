#ifndef Dimmable_h_
#define Dimmable_h_

#include "Target.h"

class Dimmable: public Target
{
    public:
        Dimmable(const char* name): Target(name) {}

        virtual void dim(int value) = 0;
        virtual ~Dimmable() {};
};

#endif  // Dimmable_h_