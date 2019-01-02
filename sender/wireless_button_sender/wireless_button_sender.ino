#include <bluefruit.h>

const int BATTERY_LED_PIN[3] = {21, 20, 3}; //R,G,B

#include "buttonhid.h"
#include "buttonplate.h"

// Battery monitoring
#define VBATPIN 31
#define MONITOR_BATTERY 0

// Encoders
#define ENCODER_1_PIN1 27
#define ENCODER_1_PIN2 30

// Running state
float lastVoltage = -10.0;
bool buttonPressed = false;

SWBButtonPlate plate;

float batteryLevel()
{
  float measured = analogRead(VBATPIN);
  measured *= 2; // cos adafruit say so
  measured *= 3.3;
  measured /= 1024;
  return measured;
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10); // for nrf52840 with native usb

  Serial.println("Neils Fake Buttons");
  Serial.println("------------------\n");

  plate.setupButtonPlate();

  // encoderOne.begin(ENCODER_1_PIN1, ENCODER_1_PIN2);
  // encoderOne.setCallback(reportEncoder);
  // encoderOne.setReporter(1);
  // encoderOne.start();
}

void loop()
{

  // int encoderResult = encoderOne.read();
  // if (encoderResult)
  // {
  //   Serial.print("Emcoder changed to: ");
  //   Serial.println(encoderResult);
  // }

  if (MONITOR_BATTERY)
  {
    float currentVolts = batteryLevel();
    if (fabs(currentVolts - lastVoltage) > 0.01)
    {
      Serial.print("Voltage: ");
      Serial.println(currentVolts, 3);
      lastVoltage = currentVolts;
    }
  }

  bool sendNewState = false;
  for (int btnNumber = 0; btnNumber < NUM_BUTTONS; btnNumber++)
  {
    if (plate.isEncoderButton(btnNumber))
      continue;
    if (!plate.isMappedButton(btnNumber))
      continue;
    int port = plate.portForButtonNumber(btnNumber);
    bool currentState = digitalRead(port) == LOW;
    sendNewState |= plate.setButtonState(btnNumber, currentState);
  }

  if (sendNewState)
  {
    plate.sendInputs();
  }
}
