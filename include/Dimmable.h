#ifndef Dimmable_h_
#define Dimmable_h_

class Dimmable
{
public:
    virtual void dim(int value) = 0;  // Reine virtuelle Methode
    virtual unsigned int getLevel() = 0;  // Reine virtuelle Methode
    virtual ~Dimmable() {}  // Virtueller Destruktor für Polymorphismus
};

#endif  // Dimmable_h_