#include <bluefruit.h>

#define NUM_BUTTONS 8

const int BATTERY_LED_PIN[3] = { 21, 20, 3 }; //R,G,B

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_CONSUMER_CONTROL,
  REPORT_ID_MOUSE,
  REPORT_ID_GAMEPAD
};

uint8_t const hid_report_descriptor[] =
{
  //------------- Keyboard Report  -------------//
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
  HID_REPORT_ID ( REPORT_ID_KEYBOARD        ),
  HID_USAGE_PAGE( HID_USAGE_PAGE_KEYBOARD ),
  // 8 bits Modifier Keys (Shfit, Control, Alt)
  HID_USAGE_MIN    ( 224                                    ),
  HID_USAGE_MAX    ( 231                                    ),
  HID_LOGICAL_MIN  ( 0                                      ),
  HID_LOGICAL_MAX  ( 1                                      ),

  HID_REPORT_COUNT ( 8                                      ),
  HID_REPORT_SIZE  ( 1                                      ),
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

  // 8 bit reserved
  HID_REPORT_COUNT ( 1                                      ),
  HID_REPORT_SIZE  ( 8                                      ),
  HID_INPUT        ( HID_CONSTANT                           ),

  // 6-byte Keycodes
  HID_USAGE_PAGE (HID_USAGE_PAGE_KEYBOARD),
  HID_USAGE_MIN    ( 0                                   ),
  HID_USAGE_MAX    ( 255                                 ),
  HID_LOGICAL_MIN  ( 0                                   ),
  HID_LOGICAL_MAX  ( 255                                 ),

  HID_REPORT_COUNT ( 6                                   ),
  HID_REPORT_SIZE  ( 8                                   ),
  HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ),

  // LED Indicator Kana | Compose | Scroll Lock | CapsLock | NumLock
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_LED                   ),
  /* 5-bit Led report */
  HID_USAGE_MIN    ( 1                                       ),
  HID_USAGE_MAX    ( 5                                       ),
  HID_REPORT_COUNT ( 5                                       ),
  HID_REPORT_SIZE  ( 1                                       ),
  HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
  /* led padding */
  HID_REPORT_COUNT ( 1                                       ),
  HID_REPORT_SIZE  ( 3                                       ),
  HID_OUTPUT       ( HID_CONSTANT                            ),
  HID_COLLECTION_END,

  //------------- Consumer Control Report -------------//
  HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER    ),
  HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
  HID_REPORT_ID( REPORT_ID_CONSUMER_CONTROL ),
  HID_LOGICAL_MIN  ( 0x00                                ),
  HID_LOGICAL_MAX_N( 0x03FF, 2                           ),
  HID_USAGE_MIN    ( 0x00                                ),
  HID_USAGE_MAX_N  ( 0x03FF, 2                           ),
  HID_REPORT_COUNT ( 1                                   ),
  HID_REPORT_SIZE  ( 16                                  ),
  HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ),
  HID_COLLECTION_END,

  //------------- Mouse Report: buttons + dx + dy + scroll + pan -------------//
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE    ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
  HID_REPORT_ID( REPORT_ID_MOUSE        ),
  HID_USAGE      (HID_USAGE_DESKTOP_POINTER ),
  HID_COLLECTION ( HID_COLLECTION_PHYSICAL  ),
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
  HID_USAGE_MIN    ( 1                                      ),
  HID_USAGE_MAX    ( 5                                      ),
  HID_LOGICAL_MIN  ( 0                                      ),
  HID_LOGICAL_MAX  ( 1                                      ),

  HID_REPORT_COUNT ( 5                                      ), /* Forward, Backward, Middle, Right, Left */
  HID_REPORT_SIZE  ( 1                                      ),
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

  HID_REPORT_COUNT ( 1                                      ),
  HID_REPORT_SIZE  ( 3                                      ),
  HID_INPUT        ( HID_CONSTANT                           ), /* 5 bit padding followed 3 bit buttons */

  HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP ),
  HID_USAGE        ( HID_USAGE_DESKTOP_X                    ),
  HID_USAGE        ( HID_USAGE_DESKTOP_Y                    ),
  HID_LOGICAL_MIN  ( 0x81                                   ), /* -127 */
  HID_LOGICAL_MAX  ( 0x7f                                   ), /* 127  */

  HID_REPORT_COUNT ( 2                                      ), /* X, Y position */
  HID_REPORT_SIZE  ( 8                                      ),
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* relative values */

  HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL                ), /* mouse scroll */
  HID_LOGICAL_MIN ( 0x81                                   ), /* -127 */
  HID_LOGICAL_MAX ( 0x7f                                   ), /* 127  */
  HID_REPORT_COUNT( 1                                      ),
  HID_REPORT_SIZE ( 8                                      ), /* 8-bit value */
  HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* relative values */

  HID_USAGE_PAGE  ( HID_USAGE_PAGE_CONSUMER ),
  HID_USAGE_N     ( HID_USAGE_CONSUMER_AC_PAN, 2           ), /* Horizontal wheel scroll */
  HID_LOGICAL_MIN ( 0x81                                   ), /* -127 */
  HID_LOGICAL_MAX ( 0x7f                                   ), /* 127  */
  HID_REPORT_COUNT( 1                                      ),
  HID_REPORT_SIZE ( 8                                      ), /* 8-bit value */
  HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* relative values */
  HID_COLLECTION_END,
  HID_COLLECTION_END,

#if 1
  //------------- Gamepad Report -------------//
  /* Byte 0: 4 pad | 2 Y-axis | 2 X-axis
     Byte 1: Button7-Button0
  */
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
  HID_REPORT_ID ( REPORT_ID_GAMEPAD      ),
  HID_USAGE      (HID_USAGE_DESKTOP_POINTER ),
  HID_COLLECTION ( HID_COLLECTION_PHYSICAL  ),
  // X,Y joystick
  HID_USAGE    ( HID_USAGE_DESKTOP_X                    ),
  HID_USAGE    ( HID_USAGE_DESKTOP_Y                    ),
  HID_LOGICAL_MIN ( 0xFF                                   ), /* -1 */
  HID_LOGICAL_MAX ( 0x01                                   ), /* 1  */
  HID_REPORT_COUNT( 2                                      ), /* X, Y position */
  HID_REPORT_SIZE ( 2                                      ), /* 2-bit value */
  HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* input values */
  HID_COLLECTION_END,

  /* X,Y padding */
  HID_REPORT_COUNT ( 4                                       ),
  HID_REPORT_SIZE  ( 1                                       ),
  HID_INPUT        ( HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

  // Buttons
  HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
  HID_USAGE_MIN    ( 1                                      ),
  HID_USAGE_MAX    ( 8                                      ),
  HID_LOGICAL_MIN  ( 0                                      ),
  HID_LOGICAL_MAX  ( 1                                      ),
  HID_REPORT_COUNT ( 8                                      ),    // Keyboard
  HID_REPORT_SIZE  ( 1                                      ),
  HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
  HID_COLLECTION_END
#endif
};
//uint8_t const hid_report_descriptor[] =
//{
//  //------------- Gamepad Report -------------//
//  /* Byte 0: 4 pad | 2 Y-axis | 2 X-axis
//   * Byte 1: Button7-Button0
//   */
//  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
//  HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD  ),
//  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
//    HID_REPORT_ID ( REPORT_ID_GAMEPAD      ),
//    HID_USAGE      (HID_USAGE_DESKTOP_POINTER ),
//
//    // Buttons
//    HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
//      HID_USAGE_MIN    ( 1                                      ),
//      HID_USAGE_MAX    ( NUM_BUTTONS                                      ),
//      HID_LOGICAL_MIN  ( 0                                      ),
//      HID_LOGICAL_MAX  ( 1                                      ),
//      HID_REPORT_COUNT ( NUM_BUTTONS                                      ),    // Keyboard
//      HID_REPORT_SIZE  ( 1                                      ),
//      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
//  HID_COLLECTION_END
//};

class SWBButtonHid : public BLEHidGeneric {
  public:
    SWBButtonHid(int numButtons) : _numButtons(numButtons), BLEHidGeneric(4, 0, 0) {
      _mse_buttons = 0;
    }

    err_t begin() {
      uint16_t inputLen [] = { sizeof(hid_keyboard_report_t),  sizeof(hid_consumer_control_report_t), sizeof(hid_mouse_report_t), sizeof(hid_gamepad_report_t) };
      uint16_t outputLen[] = {0};

      setReportLen(inputLen, outputLen, NULL);
      setReportMap(hid_report_descriptor, sizeof(hid_report_descriptor));
      VERIFY_STATUS(BLEHidGeneric::begin());

      Serial.print("Setting up for: "); Serial.print(_num_input); Serial.println(" inputs.");
      Serial.print("HID Gamepad is: "); Serial.print(sizeof(hid_gamepad_report_t)); Serial.println(" in length");

      // Attempt to change the connection interval to 11.25-15 ms when starting HID
      Bluefruit.setConnInterval(9, 12);
      return ERROR_NONE;
    }

    bool sendButton(uint8_t number) {
      hid_gamepad_report_t pad = {0};
      pad.buttons = number;

      return inputReport(REPORT_ID_GAMEPAD, &pad, sizeof(hid_gamepad_report_t));
    }

    /*------------------------------------------------------------------*/
    /* Mouse
      ------------------------------------------------------------------*/
    bool mouseReport(hid_mouse_report_t* report)
    {
      return inputReport(REPORT_ID_MOUSE, report, sizeof(hid_mouse_report_t));
    }

    bool mouseMove(int8_t x, int8_t y)
    {
      return mouseReport(_mse_buttons, x, y, 0, 0);
    }

    bool mouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan)
    {
      hid_mouse_report_t report =
      {
        .buttons = buttons,
        .x       = x,
        .y       = y,
        .wheel   = wheel,
        .pan     = pan
      };

      _mse_buttons = buttons;

      return mouseReport(&report);
    }

  private:
    int _numButtons = 0;
    uint8_t _mse_buttons;
};

BLEDis bledis;
SWBButtonHid blehid(NUM_BUTTONS);
//BLEHidAdafruit blehid;

void setup() {
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Neils Fake Buttons");
  Serial.println("------------------\n");

  Serial.println("Enter following characters");
  Serial.println("- '12345678'  to press a button\n");

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

  if (blehid.isBootMode() == 1) {
    Serial.println("Running in boot mode");
  } else {
    Serial.println("Running in report mode");
  }

  // Set up and start advertising
  startAdv();

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
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void loop() {
  // put your main code here, to run repeatedly:

  if (Serial.available())
  {
    char ch = (char) Serial.read();
    switch (ch) {
      case '1':
        if (!blehid.sendButton(1)) {
          Serial.println("Failed to send 1");
        }
        break;

      case '2':
        if (!blehid.mouseMove(0, -20)) {
          Serial.println("Failed to send mouse");
        }
        break;

      default: break;
    }
  }
}
