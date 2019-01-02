#ifndef SWB_BUTTONHID
#define SWB_BUTTONHID

#include <bluefruit.h>

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
        /* Byte 0: Button7-Button0
          * Byte 1: Button16-Button9
          */
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),
        HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),
        HID_COLLECTION(HID_COLLECTION_APPLICATION),
        HID_REPORT_ID(REPORT_ID_GAMEPAD),
        HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),

        // Buttons
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),
        HID_USAGE_MIN(1),
        HID_USAGE_MAX(NUM_BUTTONS),
        HID_LOGICAL_MIN(0),
        HID_LOGICAL_MAX(1),
        HID_REPORT_COUNT(NUM_BUTTONS), // Keyboard
        HID_REPORT_SIZE(1),
        HID_INPUT(HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),

        HID_COLLECTION_END};



class SWBButtonHid : public BLEHidGeneric
{
  public:
    SWBButtonHid() : BLEHidGeneric(4, 0, 0) {}

    err_t begin()
    {
        uint16_t inputLen[] = {0, 0, 0, sizeof(hid_button_masher_t)};
        uint16_t outputLen[] = {0};

        setReportLen(inputLen, outputLen, NULL);
        setReportMap(hid_report_descriptor2, sizeof(hid_report_descriptor2));
        VERIFY_STATUS(BLEHidGeneric::begin());

#ifdef DEBUG
        Serial.print("Setting up for: ");
        Serial.print(_num_input);
        Serial.println(" inputs.");
        Serial.print("HID report is: ");
        Serial.print(sizeof(hid_button_masher_t));
        Serial.println(" in length");
#endif
        // Attempt to change the connection interval to 11.25-15 ms when starting HID
        Bluefruit.setConnInterval(9, 12);
        return ERROR_NONE;
    }

    bool sendButtons(hid_button_masher_t *state)
    {
        return inputReport(REPORT_ID_GAMEPAD, state, sizeof(hid_button_masher_t));
    }
};

#endif
