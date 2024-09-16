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
    bool lightsTimedOut = false;
    
public:
    LimitedDarknessHandler(WebSocketEventHandler* eventHandler, unsigned long lightTimelimit = 0): DarknessHandler(eventHandler) {
        this->lightTimelimit = lightTimelimit;
    };
    
    void handleDarkness(bool isDark)
    {
        if (isDark && this->lightsTimedOut) {
            return;
        }

        if (isDark && millis() > (this->sunsetTimestamp + this->lightTimelimit)) {
            this->lightsTimedOut = true;
            this->switchOff();

            return;
        }
        
        if (!isDark && this->lightsTimedOut) {
            this->lightsTimedOut = false;
        }

        DarknessHandler::handleDarkness(isDark);
    };
};

#endif  // LimitedDarknessHandler_h_