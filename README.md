Neils "wireless buttons of doom" project.

Backgrouund
-----------

I had a wired wheel with 16 buttons, 2 shifters and 2 rotary encoders. 
I'd originally built it for road racing (iRacing), and that worked fine for a couple of years. As I began to get into dirt, the cord started getting in the way.
Grr. Cord be gone!

I looked around for existing stuff, and found a project that used 2 Arduinos in I think UART mode. The idea was you preconfigured them to talk to each other, and exposed one of them as a Joystick device on the PC. I'd list the project here but I can't find it again.

After hunting a bit I found the [adafruit feather](https://www.adafruit.com/product/3406).
This looked promising as it was an Arduino, and BLE, with Lipo charger, in a single unit.
I figured I could expose the wheel as a HID device directly over BLE, which is what I ended up doing.

Parts
-----
 - 1 x [Feather BLE](https://www.adafruit.com/product/3406) (I used the no-pins one)
 - 18 x Signal Diodes
 - A whole bunch of wire
 - The ability to solder :)
   - Soldering iron!


Diodes? Wot?
------------
What's the reason for the diodes I hear you ask?
Answer: so we can be efficient. While it is possible to have 20 odd inputs (for 16 buttons + 2 encoders) you don't have anything left.  Ask me how I know.

If you wanted to (as I did) have an LED for battery status. You're out of luck.
Enter 'matrix keyboards'. While the wiring is more complex, the result is you can use a 4x4 matrix to get 16 inputs, leaving loads of pins available for other things. Like lights! Lights are shiny. We like lights.

Here's the (pinout for the Feather](https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/device-pinout).
I used A0-A3 for my 'rows' and A4-A7 for the 'columns'.

I used the diagram from [Bald Engineer | Matrix Keyboard](https://www.baldengineer.com/arduino-keyboard-matrix-tutorial.html) when wiring things up.

I then used the [Keypad](https://github.com/Chris--A/Keypad) library to read the matrix.


Limitations / Set up for your situation
---------------------------------------

The code here is specifically for my wheel.
By that I mean the number of buttons I have, and the two encoders.
It's easy enough to change tho ... after a bit of explanation and learning on your part :-)

Here's what's important to know:
 - vars.h contains most of the configuration. Go there first.
 - buttonplate.h (go to the bottom!!!), contains the rest. Specifically: encoderConfiguration.
   -  EncoderConfig(14, 11, 16, 17) means:
    - Use pins 14, 11 for encoder input.
    - Use *button* 16 as left, *button* 17 as right. (bits 15 and 16 in the HID output)
    - If you change the number of encoders you have to change this definition.    
 - It seems HID inputs must be a multiple of 8 bits. Things didn't work for me otherwise. I'm no HID Expert, so I just made everything a multiple of 8 and got on with it.  *If you're going to change the number of buttons, this is something to keep in mind. The code already more or less takes care of this for you*.
 - Rotary Encoders are *manually configured* to appear as buttons.  So if you change the number of buttons, you might need to change the encoder button numbers as well.


I don't have any encoders
-------------------------

set NUMBER_OF_ENDCODERS to 0 in vars.h

```
#define NUMBER_OF_ENCODERS 2
```




Only later, as I was writing this, did I discover the [blue martin](https://sim-lab.eu/product/blue-marlin-wireless-joystick-device/). No idea why I didn't come across that first.
Would I have used it?  Doubt it. I don't think it'll fit nicely behind the wheel (although I couldn't spot dimensions to verify). 
It'd be nice if there was a smaller version of this board tho.  Might use it for a separate button box if I ever do one of those... then again, I've got another Feather, so ...