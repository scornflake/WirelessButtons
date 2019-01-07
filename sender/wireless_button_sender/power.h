#ifndef POWER_SWITCH
#define POWER_SWITCH

#include <Arduino.h>
#include "vars.h"

class PowerSwitch
{
  public:
    PowerSwitch(unsigned long turnOnTimeInMS, unsigned long turnOffAfterMS)
        : _gatePin(-1),
          _turnOnTimeMs(turnOnTimeInMS),
          _turnOffAfterMs(turnOffAfterMS)
    {
        _firstTime = true;
    }

    void setup()
    {
        if (_turnOffAfterMs > 0)
        {
#ifdef DEBUG_POWER_SWITCH
            Serial.printf("Will turn off automatically in %dms\n", _turnOffAfterMs);
#endif
        }
        if (canDoPowerManagement())
        {
            pinMode(_gatePin, OUTPUT);
        }
        _isPowerTurnedOn = false;
        _startTime = millis();
        _lastActivityTime = _startTime;
    }

    void setGatePin(uint8_t pin) { _gatePin = pin; }

    bool canDoPowerManagement() { return _gatePin >= 0; }

    void maybeTurnOnOrOff(bool seenActivity)
    {

        if(!canDoPowerManagement()) {
            return;
        }

        unsigned long rightNow = millis();

        // Has the user held the button for a while?
        // If so, we turn on the mosfet, to hold power up.
        unsigned long elapsedTime = rightNow - _startTime;

        if (elapsedTime > _turnOnTimeMs && !_isPowerTurnedOn && _firstTime)
        {
            if (canDoPowerManagement())
            {
                digitalWrite(_gatePin, HIGH);
            }
            _firstTime = false;
            _isPowerTurnedOn = true;
            _lastActivityTime = rightNow;
#ifdef DEBUG_POWER_SWITCH
            Serial.printf("We have been on for %dms. We're on!  Will turn off if no activity after %dms.\n", _turnOnTimeMs, _turnOffAfterMs);
#endif
        }

        if (_isPowerTurnedOn)
        {
            // Update when we last saw something happen
            if (seenActivity)
            {
                _lastActivityTime = rightNow;
#ifdef DEBUG_POWER_SWITCH
                Serial.printf("Activity seen, resetting auto power down state...\n");
#endif
            }

            // has it been some time, since something last happened?
            // If so, bye bye!
            unsigned long timeSinceActivity = rightNow - _lastActivityTime;
            if (timeSinceActivity > _turnOffAfterMs)
            {
#ifdef DEBUG_POWER_SWITCH
                Serial.printf("Should turn off. No activity in %d ms\n", _turnOffAfterMs);
                delay(10); // to allow serial to write
#endif
                putSelfToSleepOrTurnOff();
            }
        }
    }

    void putSelfToSleepOrTurnOff()
    {
        // If using the mosfet, do a hard power off
        if (canDoPowerManagement())
        {
            digitalWrite(_gatePin, LOW);
        }
        else
        {
#ifdef DEBUG_POWER_SWITCH
                Serial.printf("Cannot turn off system. No gate pin is defined to the power mosfet\n");
                delay(10);
#endif
        }
        _isPowerTurnedOn = false;
    }

  private:
    bool _isPowerTurnedOn;
    bool _firstTime;
    unsigned long _startTime;
    unsigned long _turnOnTimeMs;
    unsigned long _turnOffAfterMs;
    unsigned long _lastActivityTime;

    int _gatePin;
};

#endif