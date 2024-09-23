#ifndef Target_h_
#define Target_h_

#include <Arduino.h>

class Target 
{
    private:
        const char* name;
        bool persistable = false;

    protected:
        void setPersistable(bool persistable)
        {
            this->persistable = persistable;
        }

    public:
        Target(const char* name) {
            this->name = name;
        }

        bool isPersistable()
        {
            return this->persistable;
        }

        virtual const char* getName() 
        {
            return this->name;
        }

        virtual unsigned int getLevel() = 0;
        virtual void setLevel(unsigned int level) = 0;
};

#endif //Target_h_h