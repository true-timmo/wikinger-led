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
    
public:
    LimitedDarknessHandler(WebSocketEventHandler* eventHandler, unsigned long lightTimelimit = 0): DarknessHandler(eventHandler) {
        this->lightTimelimit = lightTimelimit;
    };
    
    void handleDarkness(bool isDark)
    {
        if (!isDark && this->sunsetTimestamp < 0) {
            this->sunsetTimestamp = 0;
        }

        if (isDark && millis() > (this->sunsetTimestamp + this->lightTimelimit)) {
            isDark = false;
        }
        
        DarknessHandler::handleDarkness(isDark);
    };
};

#endif  // LimitedDarknessHandler_h_