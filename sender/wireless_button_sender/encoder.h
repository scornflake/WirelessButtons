#ifndef __ENCODER_CONFIG
#define __ENCODER_CONFIG

struct EncoderConfig
{
  int pins[2];
  int buttonNumbers[2];

  EncoderConfig(int pin1, int pin2, int buttonOne, int buttonTwo)
  {
    pins[0] = pin1;
    pins[1] = pin2;
    buttonNumbers[0] = buttonOne;
    buttonNumbers[1] = buttonTwo;
  }
};


#endif