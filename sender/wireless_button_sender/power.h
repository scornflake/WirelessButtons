#ifndef POWER_SWITCH
#define POWER_SWITCH

#include <Arduino.h>
#include "vars.h"

class PowerSwitch
{
  public:
    PowerSwitch(unsigned long turnOffAfterMS)
        : _gatePin(-1),
          _turnOffAfterMs(turnOffAfterMS)
    {
        _firstTime = true;
    }

    void setup()
    {
        if (_turnOffAfterMs > 0)
        {
#ifdef DEBUG_POWER_SWITCH
            Serial.printf("Will turn off automatically in %dms using gate pin: %d\n", _turnOffAfterMs, _gatePin);
#endif
        }
        if (canDoPowerManagement())
        {
            pinMode(_gatePin, OUTPUT);
            // pull low immediately, to switch on the FET (assuming P channel)
            digitalWrite(_gatePin, LOW);
#ifdef DEBUG_POWER_SWITCH
        Serial.printf("Set pin %d LOW to enable the FET\n", _gatePin);
#endif          }
        _isPowerTurnedOn = true;
        _startTime = millis();
        _lastActivityTime = _startTime;
    }

    void setGatePin(uint8_t pin) { 
        _gatePin = pin; 
#ifdef DEBUG_POWER_SWITCH
        Serial.printf("Using pin %d as the gate pin\n", pin);
#endif        
    }

    bool canDoPowerManagement() { return _gatePin >= 0; }

    void maybeTurnOnOrOff(bool seenActivity)
    {
        if(!canDoPowerManagement()) {
            return;
        }

        unsigned long rightNow = millis();
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
            // Switching to HIGH in a P channel FET turns the circuit off
#ifdef DEBUG_POWER_SWITCH
            Serial.println("Switching self off...");
            delay(10); // to allow serial to write
#endif
            digitalWrite(_gatePin, HIGH);
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
    unsigned long _turnOffAfterMs;
    unsigned long _lastActivityTime;

    int _gatePin;
};

#endif