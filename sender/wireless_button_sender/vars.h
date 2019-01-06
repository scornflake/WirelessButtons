#ifndef __MASHER_VARS_
#define __MASHER_VARS_

#include "encoder.h"

// Comment this out if in dev, so that you can see Serial output.
// Its needed to NOT wait for Serial to appear, if in prod
//
// This should be defined, if building for the real button plate.
// #define PRODUCTION 1

#ifndef PRODUCTION
#define DEBUG 1
#endif

#ifdef DEBUG
// #define DEBUG_MONITOR_BATTERY 1 // set to 1 to see debug Serial output
#define DEBUG_POWER_SWITCH 1   // show info re power
#define DEBUG_BUTTON_PRESSES 1 // set to 1 to see output of button presses
#endif

/*
** User setup here 
*/

/* Power switch */
// Want to use a power switch (with mosfet)? Uncomment this.
const bool USE_POWER_SWITCH = true;

// Specify the next 3 values, to setup the gate and timeout periods
const int POWER_SWITCH_GATE_PIN = 11;                  // because I didn't actually hook pin 11 (blue) RGB up
const int AUTO_TURNOFF_MINUTES = 15;                   // How many minutes to wait with no activity, until turning off?
const int POWER_SWITCH_TURNS_ON_IF_HELD_FOR_MS = 2000; // must hold power switch for 2s to turn on wheel

// this one? well. Yeh. Leave it.
const int AUTO_TURNOFF_IF_NO_ACTIVITY_MS = (60 * AUTO_TURNOFF_MINUTES * 1000); // cos I like constants.

/* Battery Monitoring */
// What pins is the RGB LED connected to?
const int BATTERY_LED_PIN[3] = {15, 7, 11}; //R,G,B pinouts
const int VBATPIN = 31;                     // Pin from which VBAT (lipo) can be read. Adafruit nRF Feather specific.
const bool MONITOR_BATTERY = true;          // set to 1 to perform monitoring/reading of pin at all
const float MONITOR_LED_INTENSITY = 0.25f;  // Overall intensity of the LED. 0 == off, 1.0 = full on
const bool MOCK_BATTERY = false;            // If true, battery isn't ready, and fake values are sent every now and then

/* Buttons ... the Matrix */
// Vars affecting # of buttons reported over HID.
// By default, we make the buttons array on a multiple of 8 bits (rounded up).
// ::Note:: These values affect other struct sizes
const int NUMBER_OF_BUTTONS = 16;

// Our wheel has 16 buttons and 2 encoders (they take 4 inputs)
// Define a 4x4 matrix (for the 16 buttons)
const byte ROWS = 4;
const byte COLS = 4;

/* CHANGE THIS IF YOU CHANGE NUMBER_OF_BUTTONS */
char keys[ROWS * COLS] = {
    0, 1, 2, 3,
    4, 5, 6, 7,
    8, 9, 10, 11,
    12, 13, 14, 15};
byte rowPins[ROWS] = {2, 3, 4, 5};     //connect to the row pinouts of the kpd
byte colPins[COLS] = {28, 29, 12, 13}; //connect to the column pinouts of the kpd

/* Rotary Encoders */
// How many rotary encoders you have. Each requires 2 pins for input.
// SWBButtonPlate.encoderConfiguration is directly affected.
// ::Note:: These values affect other struct sizes
const int NUMBER_OF_ENCODERS = 2;

// Where are the encoders connected? (pins), and what button numbers should they output to?
EncoderConfig encoderConfiguration[NUMBER_OF_ENCODERS] = {
    EncoderConfig(14, 11, 16, 17),
    EncoderConfig(30, 27, 18, 19)};

/*
** Automagic stuff from here (shouldn't need to touch)
*/
// Adds a delay in loop(), so that the entire thing uses a bit less power
// If set to 0, no delay() is called
const int __loopDelayInMs = 1;

#define ROUND_UP(N, S) ((((N) + (S)-1) / (S)) * (S))
#define NUMBER_OF_ENCODER_OUTPUTS (NUMBER_OF_ENCODERS * 2)
#define TOTAL_OUTPUTS (NUMBER_OF_BUTTONS + NUMBER_OF_ENCODER_OUTPUTS)
#define NUM_HID_INPUTS (ROUND_UP(TOTAL_OUTPUTS, 8))
#define NUMBER_OF_BUTTON_BYTES (NUM_HID_INPUTS / 8)

/* TODO: How to put in a compiler warning if NUM_HID_INPUTS not a multiple of 8???? */

typedef ATTR_PACKED_STRUCT(struct)
{
    uint8_t buttons[NUMBER_OF_BUTTON_BYTES];
}
hid_button_masher_t;

#endif
