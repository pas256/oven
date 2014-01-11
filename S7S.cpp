// Seven segment display
#include <Wire.h> // Include the Arduino SPI library
#include "Arduino.h"
#include "S7S.h"

// Constructor
S7S::S7S(byte s7sAddress) {
  _s7sAddress = s7sAddress;
  _brightness = 254;
  _decimals = 0;
}

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void S7S::sendString(String toSend) {
  Wire.beginTransmission(_s7sAddress);
  for (int i = 0; i < 4; i++) {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void S7S::clearDisplay() {
  Wire.beginTransmission(_s7sAddress);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void S7S::setBrightness(byte value) {
  if (value == _brightness) {
    // Do nothing if the brightness is the same
    return;
  }
  
  Wire.beginTransmission(_s7sAddress);
  Wire.write(0x7A);  // Set brightness command byte
  Wire.write(value);  // brightness data byte
  Wire.endTransmission();
  
  _brightness = value;
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void S7S::setDecimals(byte decimals) {
  if (decimals == _decimals) {
    // Do nothing if there is no change
    return;
  }
  
  Wire.beginTransmission(_s7sAddress);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission();
  
  _decimals = decimals;
}

// Set the baud rate of the display
//  0  = 2400
//  1  = 4800
//  2  = 9600  (factory default)
//  3  = 14400
//  4  = 19200
//  5  = 38400
//  6  = 57600
void S7S::setBaudRate(byte value) {
  Wire.beginTransmission(_s7sAddress);
  Wire.write(0x7F); 
  Wire.write(value);
  Wire.endTransmission();
}