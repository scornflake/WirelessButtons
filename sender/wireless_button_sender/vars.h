#ifndef __MASHER_VARS_
#define __MASHER_VARS_


// Comment this out if in dev, so that you can see Serial output.
// Its needed to NOT wait for Serial to appear, if in prod
//
// This should be defined, if building for the real button plate.
//#define PRODUCTION 1

#ifndef PRODUCTION
#define DEBUG 1
#endif

// Battery monitoring
#define VBATPIN 31              // Pin to which VBAT (lipo) can be read. Adafruit nRF Feather specific.
#define MONITOR_BATTERY 0       // set to 1 to perform monitoring/reading of pin at all
#define DEBUG_MONITOR_BATTERY 0 // set to 1 to see debug Serial output
#define DEBUG_BUTTON_PRESSES 1  // set to 1 to see output of button presses

// Adds a delay in loop(), so that the entire thing uses less power
// If set to 0, no delay() is called
const int __loopDelayInMs = 0;

/* MATRIX */

// Our wheel has 16 buttons and 2 encoders (they take 4 inputs)
// Define a 4x4 matrix (for the 16 buttons)
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', '4'},
    {'5', '6', '7', '8'},
    {'9', 'a', 'b', 'c'},
    {'d', 'e', 'f', 'g'}};
byte rowPins[ROWS] = {2, 3, 4, 5};     //connect to the row pinouts of the kpd
byte colPins[COLS] = {28, 29, 12, 13}; //connect to the column pinouts of the kpd

/* ORIGINAL */

/*
** Vars affecting # of buttons reported over HID.
** ::Note:: These values affect other struct sizes
*/

// The number of rotary encoders on the button plate
// SWBButtonPlate.encoderButtons is directly affected.
#define NUMBER_OF_ENCODERS 2

// Number of buttons you want see (this must be a multiple of 8)
// If you make it more/less, you need to adjust the hid_button_masher_t type
//
// SWBButtonPlate.buttonToPortMap is directly affected
#define NUM_BUTTONS 24

typedef ATTR_PACKED_STRUCT(struct)
{
    uint8_t buttons[3]; // 3 * 8 == 24
}
hid_button_masher_t;

#endif
