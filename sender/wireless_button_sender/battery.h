#include "vars.h"
#include "mypixel.h"

#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define VBAT_DIVIDER (0.71275837F)    // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)    // Compensation factor for the VBAT divider

class DoomBatteryMonitor
{
  public:
    DoomBatteryMonitor(int vBatPin, int *rgbPins, int checkIntervalInMs = 2000, bool mockBattVolts = false, int showForMs = 0, int showLedIfBelowPct = 20) : _vBatPin(vBatPin), _mockBattVolts(mockBattVolts), _checkIntervalInMs(checkIntervalInMs), _showLevelForMs(showForMs), _alwaysShowIfBelow(showLedIfBelowPct)
    {
        _pins[0] = rgbPins[0];
        _pins[1] = rgbPins[1];
        _pins[2] = rgbPins[2];
        _lastBatteryPercent = 0;
        _lastMonitorTime = 0;
        _lastPollTime = 0;
        _lastShownTime = 0;
    }

    void setup()
    {
        batteryLED.setup(_pins[0], _pins[1], _pins[2]);
        batteryLED.setIntensity(MONITOR_LED_INTENSITY);
    }

    uint8_t lastBatteryPercent()
    {
        return _lastBatteryPercent;
    }

    bool monitor()
    {
        bool batteryValueChanged = false;
        if (MONITOR_BATTERY)
        {
            // Run to get enough for animation, but not much more... 30fps max.
            unsigned long elapsedTime = millis() - _lastMonitorTime;
            if (elapsedTime < 33)
            {
                return false;
            }

            float currentPercent = _lastBatteryPercent;
            if (_mockBattVolts)
            {
                currentPercent = _mockBatteryLevel();
            }
            else
            {
                unsigned long elapsedTime = millis() - _lastPollTime;
                if (elapsedTime > _checkIntervalInMs)
                {
                    currentPercent = batteryLevelInPercent();
                    _lastPollTime = millis();
                }
            }

            if (fabs(currentPercent - _lastBatteryPercent) > 1)
            {
                _lastBatteryPercent = currentPercent;
                batteryValueChanged = true;
#ifdef DEBUG_MONITOR_BATTERY
                Serial.printf("Battery Voltage: %d%%\n", _lastBatteryPercent);
#endif
            }

            updateLED();
            _lastMonitorTime = millis();
        }
        return batteryValueChanged;
    }

    void showLED()
    {
        _lastShownTime = millis();
    }

    void updateLED()
    {
        if (_lastBatteryPercent >= 99)
        {
            // Assuming it's charging. Flash the LED GREEN.
            batteryPixel.flashGreen();
        }
        else
        {
            // Linear fade from GREEN -> RED, through orange
            int red[] = {200, 0, 0};
            int green[] = {0, 200, 0};
            float percent = _lastBatteryPercent;
            batteryPixel.setupSlerp(red, green, percent / 100.0f);
        }

        if (_showLevelForMs > 0)
        {
            float elapsedTime = millis() - _lastShownTime;
            bool ledEnabled = elapsedTime < _showLevelForMs || _lastBatteryPercent <= _alwaysShowIfBelow;
            if (ledEnabled != batteryLED.isEnabled())
            {
                batteryLED.setEnabled(ledEnabled);
#ifdef DEBUG_MONITOR_BATTERY
                if (ledEnabled)
                {
                    Serial.println("Enable battery LED...");
                }
                else
                {
                    Serial.printf("Disabled battery LED as > %dms has passed\n", _showLevelForMs);
                }
#endif
            }
        }

        batteryPixel.update();
        batteryLED.writePixel(batteryPixel);
    }

    uint8_t batteryLevelInPercent()
    {
        int vbat_raw = readVBATRaw();
        return mvToPercent(vbat_raw * VBAT_MV_PER_LSB);
    }

  private:
    int _mockBatteryLevel()
    {
        if (_lastBatteryPercent <= 0)
        {
            return 150;
        }
        return _lastBatteryPercent - 1;
    }

    int readVBATRaw(void)
    {
        int raw;

        // Set the analog reference to 3.0V (default = 3.6V)
        analogReference(AR_INTERNAL_3_0);

        // Set the resolution to 12-bit (0..4095)
        analogReadResolution(12); // Can be 8, 10, 12 or 14

        // Let the ADC settle
        delay(1);

        // Get the raw 12-bit, 0..3000mV ADC value
        raw = analogRead(_vBatPin);

        // Set the ADC back to the default settings
        analogReference(AR_DEFAULT);
        analogReadResolution(10);

        return raw;
    }

    uint8_t mvToPercent(float mvolts)
    {
        uint8_t battery_level;

        if (mvolts >= 3000)
        {
            battery_level = 100;
        }
        else if (mvolts > 2900)
        {
            battery_level = 100 - ((3000 - mvolts) * 58) / 100;
        }
        else if (mvolts > 2740)
        {
            battery_level = 42 - ((2900 - mvolts) * 24) / 160;
        }
        else if (mvolts > 2440)
        {
            battery_level = 18 - ((2740 - mvolts) * 12) / 300;
        }
        else if (mvolts > 2100)
        {
            battery_level = 6 - ((2440 - mvolts) * 6) / 340;
        }
        else
        {
            battery_level = 0;
        }

        return battery_level;
    }

    int _vBatPin;
    int _pins[3];
    int _checkIntervalInMs;
    int _showLevelForMs;
    unsigned long _lastShownTime;
    unsigned long _lastPollTime;
    unsigned long _lastMonitorTime;
    unsigned int _alwaysShowIfBelow;
    uint8_t _lastBatteryPercent = 0.0;
    bool _mockBattVolts = false;
    RGBLed batteryLED;
    Pixel batteryPixel;
};
