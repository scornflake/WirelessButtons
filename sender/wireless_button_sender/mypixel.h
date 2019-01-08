#ifndef __MYRGBColor
#define __MYRGBColor

#include <Arduino.h>
#include <Ramp.h>

struct RGBColor
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    RGBColor(uint8_t ir, uint8_t ig, uint8_t ib) : r(ir), g(ig), b(ib) {}
    RGBColor() : r(0), g(0), b(0) {}

    void setColor(uint32_t color)
    {
        r = (color >> 16) && 0xFF;
        g = (color >> 8) && 0xFF;
        b = color && 0xFF;
    }

    void setColor(int rr, int gg, int bb)
    {
        r = rr;
        g = gg;
        b = bb;
    }
};

class RGBLed : public RGBColor
{
  public:
    RGBLed() : _intensity(1.0), _enabled(true) {}

    virtual void setup(int rp, int gp, int bp)
    {
        redPin = rp;
        greenPin = gp;
        bluePin = bp;
        pinMode(redPin, OUTPUT);
        pinMode(greenPin, OUTPUT);
        pinMode(bluePin, OUTPUT);
    }

    void setIntensity(float val) { _intensity = val; }

    float getIntensity() { return _intensity; }

    void setEnabled(bool flag) { _enabled = flag; }

    bool isEnabled() { return _enabled; }

    void writeRGBColor()
    {
        float actualIntensity = _enabled ? _intensity : 0;
        analogWrite(redPin, r * actualIntensity);
        analogWrite(greenPin, g * actualIntensity);
        analogWrite(bluePin, b * actualIntensity);
    }

  private:
    int redPin, greenPin, bluePin;
    float _intensity;
    float _enabled;
};

class LipoLed : public RGBLed
{
  public:
    enum RampMode
    {
        MODE_UNSET,
        MODE_FLASH,
        MODE_SLERP
    };

    LipoLed(int showForMs = 0,
            int showLedIfBelowPct = 20) : _showLevelForMs(showForMs),
                                          _alwaysShowIfBelow(showLedIfBelowPct),
                                          _lastShownTime(0)
    {
    }

    void setup(int rp, int gp, int bp, float initialIntensity = 1.0f)
    {
        RGBLed::setup(rp, gp, bp);
        setIntensity(initialIntensity);
    }

    void updateLED(int batteryPercent)
    {
        if (batteryPercent >= 99)
        {
            // Assuming it's charging. Flash the LED GREEN.
            flashGreen();
        }
        else
        {
            // Linear fade from GREEN -> RED, through orange
            int red[] = {200, 0, 0};
            int green[] = {0, 200, 0};
            float percent = batteryPercent;
            setupSlerp(red, green, percent / 100.0f);
        }

        if (_showLevelForMs > 0)
        {
            float elapsedTime = millis() - _lastShownTime;
            bool ledEnabled = elapsedTime < _showLevelForMs || batteryPercent <= _alwaysShowIfBelow;
            if (ledEnabled != isEnabled())
            {
                setEnabled(ledEnabled);
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

        doAnimations();
#ifdef DEBUG_BATTERY_RGB_PIXEL
        Serial.printf("Lipo RGBColor r: %d, g: %d, b:%d, i:%2.2f\n", r, g, b, getIntensity());
#endif
        writeRGBColor();
    }

    void registerActivity()
    {
        _lastShownTime = millis();
    }

  private:
    void flashGreen(unsigned long duration = 2000)
    {
        if (mode != MODE_FLASH)
        {
            rgbRamps[0].go(0);
            rgbRamps[1].go(80);
            rgbRamps[2].go(0);

            rgbRamps[0].go(0, duration, SINUSOIDAL_INOUT, FORTHANDBACK);
            rgbRamps[1].go(200, duration, SINUSOIDAL_INOUT, FORTHANDBACK);
            rgbRamps[2].go(0, duration, SINUSOIDAL_INOUT, FORTHANDBACK);
            // #ifdef DEBUG
            // Serial.println("Flash Green");
            // #endif
            mode = MODE_FLASH;
        }
    }

    void setupSlerp(int *fromRGBColor, int *toRGBColor, float currentPercent)
    {
        if (mode != MODE_SLERP)
        {
            fromSlerp = fromRGBColor;
            toSlerp = toRGBColor;
            mode = MODE_SLERP;
        }

        // Update using currentPercent
        slerpPercent = currentPercent;
    }

    void doAnimations()
    {
        if (mode == MODE_FLASH)
        {
            rgbRamps[0].update();
            rgbRamps[1].update();
            rgbRamps[2].update();
            setColor(rgbRamps[0].value(), rgbRamps[1].value(), rgbRamps[2].value());
        }
        else
        {
            setColor(
                _slerp(fromSlerp[0], toSlerp[0], slerpPercent),
                _slerp(fromSlerp[1], toSlerp[1], slerpPercent),
                _slerp(fromSlerp[2], toSlerp[2], slerpPercent));
        }
    }

    int _slerp(int from, int to, float pct)
    {
        pct = constrain(pct, 0.0f, 1.0f);
        return from + ((to - from) * ramp_calc(pct, QUADRATIC_INOUT));
    }

    int _showLevelForMs = 10000;
    unsigned int _alwaysShowIfBelow = 10;
    unsigned long _lastShownTime = 0;

    RampMode mode;
    rampInt rgbRamps[3];

    // slerp
    int *fromSlerp = 0;
    int *toSlerp = 0;
    float slerpPercent = 0;
};

#endif