int RED_PIN = 21;
int GREEN_PIN = 20;
int BLUE_PIN = 3;

struct Pixel {
  union {
    struct {
      union {
        uint8_t r;
        uint8_t red;
      };
      union {
        uint8_t g;
        uint8_t green;
      };
      union {
        uint8_t b;
        uint8_t blue;
      };
    };
    uint8_t raw[3];
  };

  inline Pixel( uint8_t ir, uint8_t ig, uint8_t ib)  __attribute__((always_inline))
    : r(ir), g(ig), b(ib)    {}
  inline Pixel( )  __attribute__((always_inline))
    : r(0), g(0), b(0)    {}

  void setColor(uint32_t color) {
    r = (color >> 16) && 0xFF;
    g = (color >> 8) && 0xFF;
    b = color && 0xFF;
  }
};

struct Fade {
  int minValue;
  int maxValue;
  int value;
  float increment;
  int direction;

  Fade(int minV, int maxV) : minValue(minV), maxValue(maxV) {
    value = 0;
    direction = 1;
  }

  inline Fade() : Fade(0, 255) {}

  int next() {
    value = (int)(fmax(fmin((int)(value + increment * direction), maxValue), minValue));
    if (value >= maxValue || value <= minValue) {
      randomSpeed();
      direction *= -1;
    }
    return value;
  }

  void randomSpeed() {
    randomSeed(analogRead(0));
    increment = random(10, 200) / 10.0;
  }
};

class RGBLed {
  private:
    int redPin, greenPin, bluePin;
    Pixel p;

  public:
    RGBLed(int rp, int gp, int bp) : p(0, 0, 0) {
      redPin = rp;
      greenPin = gp;
      bluePin = bp;
      pinMode(redPin, OUTPUT);
      pinMode(greenPin, OUTPUT);
      pinMode(bluePin, OUTPUT);
    }

    void writePixel(Pixel &pixel) {
      analogWrite(redPin, pixel.r);
      analogWrite(greenPin, pixel.g);
      analogWrite(bluePin, pixel.b);
    }
};

class SillyFader {
  private:
    RGBLed *led;
    Pixel p;
    Fade f[3];

  public:
    SillyFader() {
      led = new RGBLed(RED_PIN, GREEN_PIN, BLUE_PIN);
      for (int i = 0; i < 3; i++) {
        f[i].randomSpeed();
      }
    }

    void superFade() {
      // put your main code here, to run repeatedly:
      p.r = f[0].next();
      p.g = f[1].next();
      p.b = f[2].next();
      led->writePixel(p);
    }
};

SillyFader *fader;
void setup() {
  // put your setup code here, to run once:
  fader = new SillyFader();
}

void loop() {
  fader->superFade();
  delay(25);
}
