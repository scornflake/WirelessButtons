#ifndef __MYPIXEL
#define __MYPIXEL

#include <Ramp.h>

struct Pixel
{
    enum RampMode
    {
        MODE_UNSET,
        MODE_FLASH,
        MODE_SLERP
    };

    uint8_t r;
    uint8_t g;
    uint8_t b;

    inline Pixel(uint8_t ir, uint8_t ig, uint8_t ib) __attribute__((always_inline))
    : r(ir), g(ig), b(ib) {}
    inline Pixel() __attribute__((always_inline))
    : r(0), g(0), b(0) {}

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

    void setupSlerp(int *fromPixel, int *toPixel, float currentPercent)
    {
        if (mode != MODE_SLERP)
        {
            fromSlerp = fromPixel;
            toSlerp = toPixel;
            mode = MODE_SLERP;
        }

        // Update using currentPercent
        slerpPercent = currentPercent;
    }

    void update()
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

#ifdef DEBUG
        // Serial.printf("Pixel r: %d, g: %d, b:%d\n", r, g, b);
#endif
    }

    int _slerp(int from, int to, float pct)
    {
        pct = constrain(pct, 0.0f, 1.0f);
        return from + ((to - from) * ramp_calc(pct, QUADRATIC_INOUT));
    }

  private:
    RampMode mode;
    rampInt rgbRamps[3];

    // slerp
    int *fromSlerp;
    int *toSlerp;
    float slerpPercent;
};

class RGBLed
{
  public:
    RGBLed() : _intensity(1.0) {}

    void setup(int rp, int gp, int bp)
    {
        redPin = rp;
        greenPin = gp;
        bluePin = bp;
        pinMode(redPin, OUTPUT);
        pinMode(greenPin, OUTPUT);
        pinMode(bluePin, OUTPUT);
    }

    void setIntensity(float val)
    {
        _intensity = val;
    }

    void writePixel(Pixel &pixel)
    {
        analogWrite(redPin, pixel.r * _intensity);
        analogWrite(greenPin, pixel.g * _intensity);
        analogWrite(bluePin, pixel.b * _intensity);
    }

  private:
    int redPin, greenPin, bluePin;
    float _intensity;
};

#endif