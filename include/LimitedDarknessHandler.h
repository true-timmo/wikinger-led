#ifndef LimitedDarknessHandler_h_
#define LimitedDarknessHandler_h_

#include <Arduino.h>
#include "DarknessHandler.h"
#include "WebSocketEventHandler.h"
#include "Led.h"

class LimitedDarknessHandler: public DarknessHandler
{
private:
    unsigned long lightTimelimit = 0;
    bool resetLimit = true;
    
public:
    LimitedDarknessHandler(unsigned long lightTimelimit = 0): DarknessHandler() {
        this->lightTimelimit = lightTimelimit;
    };
    
    bool handleDarkness(bool isDark)
    {
        if (!isDark) {
            this->resetLimit = true;
        }
        else if (this->resetLimit) {
            this->resetLimit = false;
        }
        else if (millis() > (this->sunsetTimestamp + this->lightTimelimit)) {
            isDark = false;
        }

        return DarknessHandler::handleDarkness(isDark);
    };
};

#endif  // LimitedDarknessHandler_h_