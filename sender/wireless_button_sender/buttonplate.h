
#include <bluefruit.h>
#include <RotaryEncoder.h>
// For the matrix keypad
#include <Key.h>
#include <Keypad.h>

#include "vars.h"
#include "buttonhid.h"

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
    Serial.printf("Using a %dx%d matrix for buttons\n", ROWS, COLS);
#endif

    keypad = new Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#ifdef DEBUG
    Serial.println();
    Serial.println("Setting up encoders ...");
#endif

    if (NUMBER_OF_ENCODERS > 0)
    {
      for (int i = 0; i < NUMBER_OF_ENCODERS; i++)
      {
        EncoderConfig &cfg = encoderConfiguration[i];
#ifdef DEBUG
        Serial.printf("Encoder #%d uses pins %d,%d outputting to buttons %d and %d", i, cfg.pins[0], cfg.pins[1], cfg.buttonNumbers[0], cfg.buttonNumbers[1]);
        Serial.println();
#endif
        encoders[i].begin(cfg.pins[0], cfg.pins[1]);

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
            case RELEASED:
            {
              buttonStateChanged = true;
              /* I decided to use 'int' values for the keymap.
              This means I can just cast to an int, and use that directly in a call to setButtonState 
              */
              uint8_t buttonNumber = (int)keypad->key[i].kchar;
              setButtonState(buttonNumber, keypad->key[i].kstate == PRESSED ? true : false);
              buttonStateChanged = true;

#ifdef DEBUG
              String msg = keypad->key[i].kstate == PRESSED ? "Pressed" : "Released";
              Serial.print(msg);
              Serial.printf(" button number %d", buttonNumber);
              Serial.println();
#endif
            }
            default:
              break;
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

      // Serial.printf("Result for encoder %d = %d\n", idx, result);

      // set the left/right button state
      EncoderConfig &cfg = encoderConfiguration[idx];
      encoderDidChange |= setButtonState(cfg.buttonNumbers[0], result == -1);
      encoderDidChange |= setButtonState(cfg.buttonNumbers[1], result == 1);
    }

    return encoderDidChange;
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
        Serial.printf("Setting button %d", buttonNumber);
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
  // [x][y], where x  = encoder number, y = pin number
  EncoderConfig encoderConfiguration[NUMBER_OF_ENCODERS] = {
      EncoderConfig(14, 8, 16, 17),
      EncoderConfig(30, 27, 18, 19)};
};
