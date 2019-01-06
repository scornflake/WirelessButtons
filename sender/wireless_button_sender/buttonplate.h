
#include <bluefruit.h>
#include <RotaryEncoder.h>
// For the matrix keypad
#include <Key.h>
#include <Keypad.h>

#include "vars.h"
#include "buttonhid.h"

class SWBEncoderWithHold : public SwRotaryEncoder
{
public:
  SWBEncoderWithHold(int holdInMs = 25) : _msTime(holdInMs), _lastValue(0) {}

  int readWithHold()
  {
    int value = read();
    if (value != 0)
    {
      // start the timer
      sawValueAtMillis = millis();
    }

    if (sawValueAtMillis > 0)
    {
      unsigned long diff = millis() - sawValueAtMillis;
      if (diff > _msTime)
      {
        value = 0;
      }
    }

    _lastValue = value;
    return value;
  }

  int lastValue() { return _lastValue; }

private:
  uint8_t _msTime;
  int _lastValue;
  unsigned long sawValueAtMillis;
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
#ifdef PRODUCTION
    Bluefruit.setName("Button Masher");
#else
    Bluefruit.setName("Btn Mash Debug");
#endif

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
#ifdef DEBUG
    Serial.print("Using as input: ");
#endif

    keypad = new Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
//     for (int i = 0; i < NUM_BUTTONS; i++)
//     {
//       if (isMappedButton(i))
//       {
//         int port = portForButtonNumber(i);
//         pinMode(port, INPUT_PULLUP);
// #ifdef DEBUG
//         Serial.printf("%d, ", port);
// #endif
//       }
//     }
#ifdef DEBUG
    Serial.println();
#endif

    if (NUMBER_OF_ENCODERS > 0)
    {
      for (int i = 0; i < NUMBER_OF_ENCODERS; i++)
      {
        int *pins = encoderButtons[i];
#ifdef DEBUG
        Serial.printf("Encoder #%d uses buttons %d,%d", i, pins[0], pins[1]);
        Serial.println();
#endif
        encoders[i].begin(portForButtonNumber(pins[0]), portForButtonNumber(pins[1]));

        // only needed for HwRotaryEncoder, which I decided not to use
        // when I was debugging the weirdness. Not sure Hw was the cause.
        //encoders[i].start();
      }
    }
  }

  int numberOfEncoders()
  {
    return NUMBER_OF_ENCODERS;
  }

  bool pollButtons()
  {
    String msg = "";
    bool buttonStateChanged = false;
    if (keypad)
    {
      if (keypad->getKeys())
      {
        for (int i = 0; i < LIST_MAX; i++)
        {
          if (keypad->key[i].stateChanged)
          {
            switch (keypad->key[i].kstate)
            {
            case PRESSED:
              msg = "Pressed";
              break;
            case RELEASED:
              msg = "Released";
              break;
            default:
              msg = "";
              break;
            }
            if (msg.length())
            {
              buttonStateChanged = true;
              Serial.print(msg);
              Serial.printf("%c", keypad->key[i].kchar);
              Serial.println();
            }
          }
        }
      }
    }
    return buttonStateChanged;
  }

  bool pollEncoders()
  {
    bool encoderDidChange = false;
    for (int idx = 0; idx < NUMBER_OF_ENCODERS; idx++)
    {
      int result = encoders[idx].readWithHold();

      // set the left/right button state
      encoderDidChange |= setButtonState(encoderButtons[idx][0], result == -1);
      encoderDidChange |= setButtonState(encoderButtons[idx][1], result == 1);
    }

    return encoderDidChange;
  }

  bool isMappedButton(int buttonNumber)
  {
    return portForButtonNumber(buttonNumber) > -1;
  }

  bool isEncoderButton(int buttonNumber)
  {
    for (int i = 0; i < NUMBER_OF_ENCODERS; i++)
    {
      if (encoderButtons[i][0] == buttonNumber || encoderButtons[i][1] == buttonNumber)
      {
        return true;
      }
    }
    return false;
  }

  bool isButtonPressed(int buttonNumber)
  {
    int port = portForButtonNumber(buttonNumber);
    if (port == 20)
    {
      // DFU
    }
    return digitalRead(port) == LOW;
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
#ifdef DEBUG_BUTTON_PRESSES &&DEBUG
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
  SWBEncoderWithHold encoders[NUMBER_OF_ENCODERS];
  hid_button_masher_t _state;
  Keypad *keypad = 0;

  // Encoder button numbers (easier to read!)
  // [x][y], where x  = encoder number, y = button number
  int encoderButtons[NUMBER_OF_ENCODERS][2] = {{5, 7}, {14, 15}};

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
