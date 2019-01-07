
#include <bluefruit.h>

#include "vars.h"
#include "buttonhid.h"
#include "buttonplate.h"
#include "battery.h"
#include "power.h"

PowerSwitch powerSwitch(POWER_SWITCH_GATE_PIN, POWER_SWITCH_TURNS_ON_IF_HELD_FOR_MS, AUTO_TURNOFF_IF_NO_ACTIVITY_MS);
SWBButtonPlate plate;
DoomBatteryMonitor batteryMonitor(VBATPIN, (int *)&BATTERY_LED_PIN[0], 1000, MOCK_BATTERY, SHOW_BATTERY_FOR_MS, POWER_LED_ALWAYS_SHOWS_IF_BELOW_PCT);

void plateButtonPressed(int buttonNumber, KeyState state)
{
#ifdef DEBUG
  String msg = state == PRESSED ? "Pressed" : "Released";
  Serial.print(msg);
  Serial.printf(" button number %d", buttonNumber);
  Serial.println();
#endif

  if (POWER_BTN_SHORT_PRESS_TO_SHOW_BATTERY_LEVEL)
  {
    if (buttonNumber == POWER_BTN_SHORT_PRESS_BUTTON_NUMBER && state == HOLD)
    {
      batteryMonitor.showLED();
    }
  }
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
  plate.setButtonPressCallback(plateButtonPressed);
  batteryMonitor.setup();

  if (USE_POWER_SWITCH)
  {
    powerSwitch.setup();
  }
}

void loop()
{
  if (batteryMonitor.monitor())
  {
    plate.notifyNewBatteryLevel(batteryMonitor.lastBatteryPercent());
  }

  bool sendNewState = false;

  // check buttons
  sendNewState = plate.pollButtons();

  // check Encoders
  sendNewState |= plate.pollEncoders();

  // check power
  if (USE_POWER_SWITCH)
  {
    powerSwitch.update(sendNewState);
  }

  if (sendNewState)
  {
    plate.sendInputs();
  }

  if (__loopDelayInMs > 0)
  {
    delay(__loopDelayInMs);
  }
}
