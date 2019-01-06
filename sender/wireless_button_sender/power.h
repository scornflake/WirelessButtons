#ifndef POWER_SWITCH
#define POWER_SWITCH

#include <Arduino.h>
#include "vars.h"

class PowerSwitch
{
  public:
    PowerSwitch(int gatePin, unsigned long turnOnTimeInMS, unsigned long turnOffAfterMS) : _gatePin(gatePin), _turnOnTimeMs(turnOnTimeInMS), _turnOffAfterMs(turnOffAfterMS) {}

    void setup()
    {
        pinMode(_gatePin, OUTPUT);
        _onOffState = false;
        _startTime = millis();
    }

    void update(bool seenActivity)
    {
        unsigned long rightNow = millis();

        // Has the user held the button for a while?
        // If so, we turn on the mosfet, to hold power up.
        unsigned long elapsedTime = rightNow - _startTime;
        if (elapsedTime > _turnOnTimeMs && !_onOffState)
        {
            digitalWrite(_gatePin, HIGH);
            _onOffState = true;
            _lastActivityTime = rightNow;
#ifdef DEBUG_POWER_SWITCH
            Serial.printf("We have been on for %dms. We're on!  Will turn off if no activity after %dms.\n", _turnOnTimeMs, _turnOffAfterMs);
#endif
        }

        if (_onOffState)
        {

            // Update when we last saw something happen
            if (seenActivity)
            {
                _lastActivityTime = rightNow;
            }

            // has it been some time, since something last happened?
            // If so, bye bye!
            unsigned long timeSinceActivity = rightNow - _lastActivityTime;
            if (timeSinceActivity > _turnOffAfterMs)
            {
#ifdef DEBUG_POWER_SWITCH
                Serial.printf("Turning off. No activity in %d ms\n", _turnOffAfterMs);
                delay(100); // to allow serial to write
#endif
                digitalWrite(_gatePin, LOW);
                _onOffState = false;
            }
        }
    }

  private:
    bool _onOffState;
    unsigned long _startTime;
    unsigned long _turnOnTimeMs;
    unsigned long _turnOffAfterMs;
    unsigned long _lastActivityTime;
    int _gatePin;
};

#endif