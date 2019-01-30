
#define VBAT_MV_PER_LSB (0.73242188F) // 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096
#define VBAT_DIVIDER (0.71275837F)    // 2M + 0.806M voltage divider on VBAT = (2M / (0.806M + 2M))
#define VBAT_DIVIDER_COMP (1.403F)    // Compensation factor for the VBAT divider

class BatteryLevelReader;

typedef void (*battery_level_changed_t)(BatteryLevelReader *reader, int currntPercent);

class BatteryLevelReader
{
  public:
    BatteryLevelReader(int vBatPin,
                       int checkIntervalInMs = 2000,
                       bool mockBattVolts = false) : _vBatPin(vBatPin),
                                                     _mockBattVolts(mockBattVolts),
                                                     _checkIntervalInMs(checkIntervalInMs)

    {
        _lastBatteryPercent = 0;
        _lastMonitorTime = 0;
        _lastPollTime = 0;
        _levelChangedCallback = 0;
    }

    uint8_t lastBatteryPercent() { return _lastBatteryPercent; }
    void setMonitorCallback(battery_level_changed_t cb) { _levelChangedCallback = cb; }

    bool monitor()
    {
        bool batteryValueChanged = false;
        if (MONITOR_BATTERY)
        {
            // Run to get enough for animation of the mock, but not much more... 30fps max.
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
            }

            _lastMonitorTime = millis();
        }
        if (batteryValueChanged)
        {
            if (_levelChangedCallback != 0)
            {
                _levelChangedCallback(this, _lastBatteryPercent);
            }
        }
        return batteryValueChanged;
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
    int _checkIntervalInMs;
    unsigned long _lastPollTime;
    unsigned long _lastMonitorTime;
    uint8_t _lastBatteryPercent = 0.0;
    bool _mockBattVolts = false;
    battery_level_changed_t _levelChangedCallback = 0;
};
