#ifndef Dimmable_h_
#define Dimmable_h_

class Dimmable
{
    protected:
        const char* name;

    public:
        virtual const char* getName() 
        {
            return this->name;
        };

        Dimmable(const char* name) {
            this->name = name;
        }

        virtual void dim(int value) = 0;
        virtual unsigned int getLevel() = 0;
        virtual void setLevel(unsigned int level) = 0;
        virtual ~Dimmable() {};
};

#endif  // Dimmable_h_