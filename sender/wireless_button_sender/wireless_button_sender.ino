#include <bluefruit.h>

#define NUM_BUTTONS 24
#define VBATPIN A9

const int BATTERY_LED_PIN[3] = {21, 20, 3}; //R,G,B

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_CONSUMER_CONTROL,
  REPORT_ID_MOUSE,
  REPORT_ID_GAMEPAD
};

uint8_t const hid_report_descriptor2[] =
{
 //------------- Gamepad Report -------------//
 /* Byte 0: 4 pad | 2 Y-axis | 2 X-axis
  * Byte 1: Button7-Button0
  * Byte 2: Button16-Button9
  */
 HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
 HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  ),
 HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
   HID_REPORT_ID ( REPORT_ID_GAMEPAD      ),
   HID_USAGE      (HID_USAGE_DESKTOP_GAMEPAD ),

   // Buttons
   HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
     HID_USAGE_MIN    ( 1                                      ),
     HID_USAGE_MAX    ( NUM_BUTTONS                                      ),
     HID_LOGICAL_MIN  ( 0                                      ),
     HID_LOGICAL_MAX  ( 1                                      ),
     HID_REPORT_COUNT ( NUM_BUTTONS                                      ),    // Keyboard
     HID_REPORT_SIZE  ( 1                                      ),
     HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
 HID_COLLECTION_END
};

typedef ATTR_PACKED_STRUCT(struct) 
{
  uint8_t buttons1;
  uint8_t buttons2;
  uint8_t buttons3;
} hid_button_masher_t;

class SWBButtonHid : public BLEHidGeneric
{
public:
  SWBButtonHid(int numButtons) : _numButtons(numButtons), BLEHidGeneric(4, 0, 0)
  {
    _mse_buttons = 0;
  }

  err_t begin()
  {
    uint16_t inputLen[] = {0, 0, 0, sizeof(hid_button_masher_t)};
    uint16_t outputLen[] = {0};

    setReportLen(inputLen, outputLen, NULL);
    setReportMap(hid_report_descriptor2, sizeof(hid_report_descriptor2));
    VERIFY_STATUS(BLEHidGeneric::begin());

    Serial.print("Setting up for: ");
    Serial.print(_num_input);
    Serial.println(" inputs.");
    Serial.print("HID report is: ");
    Serial.print(sizeof(hid_button_masher_t));
    Serial.println(" in length");

    // Attempt to change the connection interval to 11.25-15 ms when starting HID
    Bluefruit.setConnInterval(9, 12);
    return ERROR_NONE;
  }

  bool sendButton(uint8_t number)
  {
    hid_button_masher_t pad = {0};
    pad.buttons1 = number;
    pad.buttons2 = pad.buttons3 = 0;

    return inputReport(REPORT_ID_GAMEPAD, &pad, sizeof(hid_button_masher_t));
  }

private:
  int _numButtons = 0;
  uint8_t _mse_buttons;
};

const int inputButtonPin = 2;
BLEDis bledis;
SWBButtonHid blehid(NUM_BUTTONS);
//BLEHidAdafruit blehid;

float batteryLevel() {
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

  Bluefruit.begin();

  // HID Device can have a min connection interval of 9*1.25 = 11.25 ms
  Bluefruit.setConnInterval(9, 16); // min = 9*1.25=11.25 ms, max = 16*1.25=20ms

  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(0);
  Bluefruit.setName("Button Masher");

  // Configure and Start Device Information Service
  bledis.setManufacturer("shinywhiteox");
  bledis.setModel("ButtonMasher Max 99");
  bledis.begin();

  // BLE HID
  blehid.begin();

  if (blehid.isBootMode() == 1)
  {
    Serial.println("Running in boot mode");
  }
  else
  {
    Serial.println("Running in report mode");
  }

  // Set up and start advertising
  startAdv();

  pinMode(inputButtonPin, INPUT_PULLUP);
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  //  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_GAMEPAD);
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

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

uint8_t _buttons = 0;
bool buttonPressed = false;

void loop()
{
  // put your main code here, to run repeatedly:

  bool newButtonState = digitalRead(inputButtonPin) == LOW;
  if (newButtonState != buttonPressed)
  {
    buttonPressed = newButtonState;
    if (buttonPressed)
    {
      Serial.println("Button pressed");
    }
    else
    {
      Serial.println("Button released");
    }

    if(!blehid.sendButton((uint8_t)buttonPressed)) {
      Serial.println("Failed to send new button state over BLE");
    }
  }
}
