#include <Key.h>
#include <Keypad.h>

// For test
const byte ROWS = 2; //four rows
const byte COLS = 2; //three columns
char keys[ROWS][COLS] = {
    {'1', '2'},
    {'3', '4'}};
byte rowPins[ROWS] = {2, 3}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {4, 5}; //connect to the column pinouts of the kpd

// For real
// const byte ROWS = 4; //four rows
// const byte COLS = 5; //three columns
// char keys[ROWS][COLS] = {
//     {'1', '2', '3', '4', '5'},
//     {'6', '7', '9', 'a', 'b'},
//     {'c', 'd', 'e', 'f', 'g'},
//     {'h', 'i', 'j', 'k', 'l'}};
// byte rowPins[ROWS] = {2, 3, 4, 5};            //connect to the row pinouts of the kpd
// byte colPins[COLS] = {28, 29, 12, 13, 14, 8}; //connect to the column pinouts of the kpd

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10); // for nrf52840 with native usb

    Serial.println("Matrix of DOOM");
    Serial.println("--------------\n");
    kpd.setDebounceTime(1);
}

unsigned long loopCount;
unsigned long startTime;
String msg;

void loop()
{
    loopCount++;
    if ((millis() - startTime) > 5000)
    {
        Serial.print("Average loops per second = ");
        Serial.println(loopCount / 5);
        startTime = millis();
        loopCount = 0;
    }

    // Fills kpd.key[ ] array with up-to 10 active keys.
    // Returns true if there are ANY active keys.
    if (kpd.getKeys())
    {
        for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
        {
            if (kpd.key[i].stateChanged) // Only find keys that have changed state.
            {
                switch (kpd.key[i].kstate)
                { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
                case PRESSED:
                    msg = " PRESSED.";
                    break;
                case HOLD:
                    msg = " HOLD.";
                    break;
                case RELEASED:
                    msg = " RELEASED.";
                    break;
                case IDLE:
                    msg = " IDLE.";
                }
                Serial.print("Key ");
                Serial.print(kpd.key[i].kchar);
                Serial.println(msg);
            }
        }
    }
}
