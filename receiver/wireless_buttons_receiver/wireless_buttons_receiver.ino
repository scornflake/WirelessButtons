#include <Mouse.h>



/* Base (connected with USB to a PC)
   for wireless bluetooth button/shifter system using
   two Teensy 3.2 microcontrollers and 2 HC-05 bluetooth modules.

   Pinout for this base unit:
  0 serial rx
  1 serial tx
  2 bluetooth status
  22 battery warning led
  23 power /connection status led
*/

#include <bluefruit.h>
#include <usb.h>
#include <Joystick.h>

// used pin definitions
#define btStatus 2
#define batteryLed 22
#define connectionLed 23

// define led brighness here in percentage
#define LED_BRIGHTNESS 5
// for 82-120 ohm series resistor and small 3mm red/green through-hole
// led, 5 % seems fine.
#define ledOutputValue 255*LED_BRIGHTNESS/100

// some status variables
boolean batteryLow = false;
byte batteryLedStatus = 0;

boolean btConnected = false;
byte connectionLedStatus = 0;

#include <elapsedMillis.h>

long const int WaitIndicationInterval = 200;
elapsedMillis lastWaitIndicationUpdate;


long const int WaitBatteryInterval = 1000;
elapsedMillis lastBatteryIndicationUpdate;

#ifndef digitalWriteFast
#define digitalWriteFast digitalWrite
#endif

void setup() {

  // I/O directions
  pinMode(btStatus, INPUT);
  pinMode(batteryLed, OUTPUT);
  pinMode(connectionLed, OUTPUT);

  // wake-up blinking

  digitalWriteFast(connectionLed, HIGH);
  digitalWriteFast(batteryLed, HIGH);
  delay(1000);
  digitalWriteFast(connectionLed, LOW);
  digitalWriteFast(batteryLed, LOW);

  // set all joystick things to known positions
  Joystick.useManualSend(true); // enable this to get simultaneous button presses if needed
  Joystick.X(512);            // "value" is from 0 to 1023
  Joystick.Y(512);            //   512 is resting position
  Joystick.Z(0);
  Joystick.Zrotate(0);
  Joystick.sliderLeft(0);
  Joystick.sliderRight(0);

  // all buttons off
  for (int i = 1; i < 33; i++) {
    Joystick.button(i, 0);
  }
  Joystick.send_now();


  // start serial even without bluetooth connection
  Serial1.begin(57600); // use same baud rate as you have set your BT modules
  Serial1.flush();
  Serial1.flush(); // flush twice to make sure

  // reset these to known value
  // not needed using elapsedMillis.
  // lastWaitIndicationUpdate = millis();
  // lastBatteryIndicationUpdate = millis();

  pinMode(1, INPUT_PULLUP);

}

void loop() {
  Joystick.X(512);            // "value" is from 0 to 1023
  Joystick.Y(512);            //   512 is resting position
  Joystick.Z(0);
  Joystick.Zrotate(0);
  Joystick.sliderLeft(0);
  Joystick.sliderRight(0);
  // Check Bluetooth status
  if (digitalReadFast(btStatus)) {
    analogWrite(connectionLed, ledOutputValue);
    btConnected = true;
  } else {
    btConnected = false;
    analogWrite(batteryLed, 0); // no connection, can't say the battery is low.
    batteryLow = false;
  }

  if (!digitalRead(1)) {
    Joystick.button(1, 1);
  } else {
    Joystick.button(1, 0);
  }
        Joystick.send_now();

  if (btConnected) {
    if (Serial1.available()) {
      char buttons = Serial1.read();


      // check high bit
      if (!bitRead(buttons, 7)) {
        //do stuff for buttons 0-6 (joystick 1-7)
        if (bitRead(buttons, 0)) {
          Joystick.button(1, 1);
        } else {
          Joystick.button(1, 0);
        }
        if (bitRead(buttons, 1)) {
          Joystick.button(2, 1);
        } else {
          Joystick.button(2, 0);
        }
        if (bitRead(buttons, 2)) {
          Joystick.button(3, 1);
        } else {
          Joystick.button(3, 0);
        }
        if (bitRead(buttons, 3)) {
          Joystick.button(4, 1);
        } else {
          Joystick.button(4, 0);
        }
        if (bitRead(buttons, 4)) {
          Joystick.button(5, 1);
        } else {
          Joystick.button(5, 0);
        }
        if (bitRead(buttons, 5)) {
          Joystick.button(6, 1);
        } else {
          Joystick.button(6, 0);
        }
        if (bitRead(buttons, 6)) {
          Joystick.button(7, 1);
        } else {
          Joystick.button(7, 0);
        }
        Joystick.send_now();

      } else {
        // do stuff for buttons 7-x (joystick 8-10)
        // (my own system has 8 buttons + 2 shifter paddles totaling 10)
        if (bitRead(buttons, 0)) {
          Joystick.button(8, 1);
        } else {
          Joystick.button(8, 0);
        }
        if (bitRead(buttons, 1)) {
          Joystick.button(9, 1);
        } else {
          Joystick.button(9, 0);
        }
        if (bitRead(buttons, 2)) {
          Joystick.button(10, 1);
        } else {
          Joystick.button(10, 0);
        }

        // there are some spare bits. This uses one of them for battery status reporting.
        // get battery status
        if (bitRead(buttons, 6)) { // bit 6 && 7 are both ones when battery low
          batteryLow = true;
        } else {
          batteryLow = false;
        }
        Joystick.send_now();
      }
    }
    // periodic battery status blinking
    runBatteryIndication();
  } else {
    // periodic bluetooth connection status
    runWaitBtIndication();
  }
}

void runWaitBtIndication() {
  if (lastWaitIndicationUpdate > WaitIndicationInterval) {
    lastWaitIndicationUpdate = 0;
    if (connectionLedStatus) {
      analogWrite(connectionLed, ledOutputValue);
    } else {
      analogWrite(connectionLed, 0);
    }
    connectionLedStatus = !connectionLedStatus;
  }
}

void runBatteryIndication() {
  if (lastBatteryIndicationUpdate > WaitBatteryInterval) {
    lastBatteryIndicationUpdate = 0;
    // blink status led at known interval when battery is low.
    if (batteryLow) {
      if (batteryLedStatus) {
        analogWrite(batteryLed, ledOutputValue);
      } else {
        analogWrite(batteryLed, 0);
      }
      batteryLedStatus = !batteryLedStatus;
    } else {
      //make sure it does not stay on!
      analogWrite(batteryLed, 0);
    }
  }
}
