#ifndef Target_h_
#define Target_h_

#include <Arduino.h>

class Target 
{
    private:
        const char* name;

    public:
        Target(const char* name) {
            this->name = name;
        }

        virtual const char* getName() 
        {
            return this->name;
        }

        virtual unsigned int getLevel() = 0;
        virtual void setLevel(unsigned int level) = 0;
};

#endif //Target_h_h