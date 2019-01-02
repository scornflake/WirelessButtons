
#include <bluefruit.h>
#include <RotaryEncoder.h>
#include "buttonhid.h"

#define DEBUG_BUTTON_PRESSES 1
#define NUMBER_OF_ENCODERS 2

class SWBEncoderWithHold : HwRotaryEncoder
{
public:
  SWBEncoderWithHold(int holdInMs = 25) : _msTime(holdInMs) {}

private:
  HwRotaryEncoder encoder;
  uint8_t _msTime;
};

class SWBButtonPlate
{
public:
  SWBButtonPlate()
  {
    varclr(&_state);
  }

  void setupButtonPlate()
  {
    Bluefruit.begin();

    // HID Device can have a min connection interval of 9*1.25 = 11.25 ms
    Bluefruit.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms

    // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
    Bluefruit.setTxPower(0);
    Bluefruit.setName("Button Masher");

    // Configure and Start Device Information Service
    bledis.setManufacturer("shinywhitebox");
    bledis.setModel("ButtonMasher Max 99");
    bledis.begin();

    setupButtonInputs();

    // BLE HID
    hid.begin();
    startAdvertising();
  }

  void setupButtonInputs()
  {
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
      if (isMappedButton(i))
      {
        pinMode(portForButtonNumber(i), INPUT_PULLUP);
      }
    }

    if(NUMBER_OF_ENCODERS > 0) {
      for(int i = 0; i < NUMBER_OF_ENCODERS; i++) {
        int *pins = encoders[i];
        Serial.printf("Encoder #%d uses buttons %d,%d", i, pins[0], pins[1]);
        Serial.println();
      }
    }
  }

  bool isMappedButton(int buttonNumber)
  {
    return portForButtonNumber(buttonNumber) > -1;
  }

  bool isEncoderButton(int buttonNumber)
  {
    for (int i = 0; i < NUMBER_OF_ENCODERS; i++)
    {
      int *encoderButtons = &encoders[i][0];
      if(encoderButtons[0] == buttonNumber || encoderButtons[1] == buttonNumber) {
        return true;
      }
    }
    return false;
  }

  int portForButtonNumber(uint8_t buttonNumber)
  {
    return buttonToPortMap[buttonNumber];
  }

  bool setButtonState(uint8_t buttonNumber, bool state)
  {
    int arrayIndex = buttonNumber / 8;
    int arrayOffset = buttonNumber % 8;

    if (arrayIndex >= 0 || arrayIndex < 3)
    {
      int existingValue = (_state.buttons[arrayIndex] >> arrayOffset) & 0x1;
      if (existingValue != state)
      {
#ifdef DEBUG_BUTTON_PRESSES
        Serial.printf("Setting button %d (port %d)", buttonNumber, portForButtonNumber(buttonNumber));
        Serial.printf(", array: %d offset %d", arrayIndex, arrayOffset);
        Serial.print(", to state: ");
        state ? Serial.print("ON") : Serial.print("OFF");
        Serial.println();
#endif

        if (state)
        {
          _state.buttons[arrayIndex] |= (1 << arrayOffset);
        }
        else
        {
          _state.buttons[arrayIndex] &= ~(1 << arrayOffset);
        }

        return true;
      }
    }

    return false;
  }

  void sendInputs()
  {
    hid.sendButtons(&_state);
  }

private:
  void startAdvertising(void)
  {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();
    Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
    // Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);

    // Include BLE HID service
    Bluefruit.Advertising.addService(hid);

    // There is enough room for 'Name' in the advertising packet
    Bluefruit.Advertising.addName();

    /* Start Advertising
     - Enable auto advertising if disconnected
     - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
     - Timeout for fast mode is 30 seconds
     - Start(timeout) with timeout = 0 will advertise forever (until connected)

     For recommended advertising interval
     https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
  }

private:
  SWBButtonHid hid; // 24 buttons over BLE
  BLEDis bledis;
  SWBEncoderWithHold encoderOne();
  SWBEncoderWithHold encoderTwo();
  hid_button_masher_t _state;

  // Encoder button numbers (easier to read!)
  // [x][y], where x  = encoder number, y = button number
  int encoders[NUMBER_OF_ENCODERS][2] = {{5, 7}, {14, 15}};

  //  Button 'integer' numbers (0-19) => pin input ports
  int buttonToPortMap[NUM_BUTTONS] = {
      2, // 0
      3, // 1
      4, // 2..
      5,
      28,
      29,
      12,
      13,
      14,
      8,
      6,
      20,
      25,
      26,
      27,
      30,
      11,
      7,
      15,
      16,
      -1,
      -1,
      -1,
      -1,
  };
};
