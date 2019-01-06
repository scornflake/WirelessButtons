#include <bluefruit.h>

const int BATTERY_LED_PIN[3] = {21, 20, 3}; //R,G,B

#include "vars.h"
#include "buttonhid.h"
#include "buttonplate.h"

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

#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    delay(10); // for nrf52840 with native usb

  Serial.println("Neils Button Masher");
  Serial.println("-------------------\n");
  Serial.printf("Number of buttons: %d\n", NUMBER_OF_BUTTONS);
  Serial.printf("Number of encoders: %d\n", NUMBER_OF_ENCODERS);
  Serial.printf("Total number of HID Inputs: %d\n", NUM_HID_INPUTS);
  Serial.printf("Num bytes for button HID struct: %d\n", NUMBER_OF_BUTTON_BYTES);
  Serial.println();
#endif

  plate.setupButtonPlate();
}

void loop()
{
  if (MONITOR_BATTERY)
  {
    float currentVolts = batteryLevel();
    if (fabs(currentVolts - lastVoltage) > 0.01)
    {
#ifdef DEBUG_MONITOR_BATTERY
      Serial.print("Voltage: ");
      Serial.println(currentVolts, 3);
#endif
      lastVoltage = currentVolts;
    }
  }

  bool sendNewState = false;

  // check buttons
  sendNewState = plate.pollButtons();

  // check Encoders
  sendNewState |= plate.pollEncoders();

  if (sendNewState)
  {
    plate.sendInputs();
  }

  if (__loopDelayInMs > 0)
  {
    delay(__loopDelayInMs);
  }
}
