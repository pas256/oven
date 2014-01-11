// Serial 7 Segment display library using only I2C

#ifndef S7S_H
#define S7S_H

#include "Arduino.h"

class S7S {
  public:
    S7S(byte s7sAddress);
//    ~S7S();
    
    // This custom function works somewhat like a serial.print.
    //  You can send it an array of chars (string) and it'll print
    //  the first 4 characters in the array.
    void sendString(String toSend);

    // Send the clear display command (0x76)
    //  This will clear the display and reset the cursor
    void clearDisplay();

    // Set the displays brightness. Should receive byte with the value
    //  to set the brightness to
    //  dimmest------------->brightest
    //     0--------127--------255
    void setBrightness(byte value);

    // Turn on any, none, or all of the decimals.
    //  The six lowest bits in the decimals parameter sets a decimal 
    //  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
    //  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
    void setDecimals(byte decimals);
    
    // Set the baud rate of the display
    //  0  = 2400
    //  1  = 4800
    //  2  = 9600  (factory default)
    //  3  = 14400
    //  4  = 19200
    //  5  = 38400
    //  6  = 57600
    void setBaudRate(byte value);
    
  private:
    byte _s7sAddress;
    byte _brightness;
    byte _decimals;
};

#endif
