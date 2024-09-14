#ifndef Dimmable_h_
#define Dimmable_h_

class Dimmable
{
public:
    virtual void dim(int value) = 0;
    virtual unsigned int getLevel() = 0;
    virtual void setLevel(unsigned int) = 0;
    virtual ~Dimmable() {}  // Virtueller Destruktor für Polymorphismus
};

#endif  // Dimmable_h_