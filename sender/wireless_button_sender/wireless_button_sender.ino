
#include <bluefruit.h>

#include "vars.h"
#include "buttonhid.h"
#include "buttonplate.h"
#include "battery.h"
#include "power.h"
#include "mypixel.h"

SWBButtonPlate plate;
PowerSwitch powerSwitch(POWER_SWITCH_TURNS_ON_IF_HELD_FOR_MS, AUTO_TURNOFF_IF_NO_ACTIVITY_MS);
BatteryLevelReader batteryMonitor(VBATPIN, 1000, MOCK_BATTERY);
LipoLed batteryLED(SHOW_BATTERY_FOR_MS, POWER_LED_ALWAYS_SHOWS_IF_BELOW_PCT);

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

  // only attempts to control the mosfet if the gate pin is set.
  // it'll still do other stuff, like wait for inactivity and sleep the device
  if (USE_POWER_SWITCH)
  {
    powerSwitch.setGatePin(POWER_SWITCH_GATE_PIN);
  }

  powerSwitch.setup();
#ifdef DEBUG
  Serial.println("Setup battery monitor...");
#endif
  batteryMonitor.setMonitorCallback(batteryLevelChanged);
#ifdef DEBUG
  Serial.println("Setup battery led...");
#endif
  batteryLED.setup(BATTERY_LED_PIN[0], BATTERY_LED_PIN[1], BATTERY_LED_PIN[2], MONITOR_LED_INTENSITY);

#ifdef DEBUG
  Serial.println("Setup complete");
#endif
}

void batteryLevelChanged(BatteryLevelReader *reader, int level)
{
#ifdef DEBUG_BATTERY_NOTIFICATIONS
  Serial.printf("Battery level at: %d\n", level);
#endif
  plate.notifyNewBatteryLevel(level);
}

void plateButtonPressed(int buttonNumber, KeyState state)
{
#ifdef DEBUG
  String msg = state == PRESSED ? "Pressed" : "Released";
  Serial.print(msg);
  Serial.printf(" button number %d\n", buttonNumber);
#endif

  if (POWER_BTN_SHORT_PRESS_TO_SHOW_BATTERY_LEVEL)
  {
    if (buttonNumber == POWER_BTN_SHORT_PRESS_BUTTON_NUMBER && state == HOLD)
    {
      batteryLED.registerActivity();
    }
  }
}

void loop()
{
  bool sendNewState = false || ALWAYS_SEND_STATE;

  // check buttons
  sendNewState = plate.pollButtons();

  // check Encoders
  sendNewState |= plate.pollEncoders();

  // check power
  // If we're sending new state, it's because a button has been pressed.
  // Use this to also tell the power switch that the device is in use
  powerSwitch.maybeTurnOnOrOff(sendNewState);

  // Monitor battery levels. Callback is fired if it's changed.
  // The level is then set to the plate (it sends it over BLE).
  // The LED is updated all the time, to assure animations!
  batteryMonitor.monitor();
  batteryLED.updateLED(batteryMonitor.lastBatteryPercent());

  if (sendNewState)
  {
    plate.sendInputs();
  }

  if (__loopDelayInMs > 0)
  {
    delay(__loopDelayInMs);
  }
}
