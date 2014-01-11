// Source: http://stackoverflow.com/questions/8507885/shift-hue-of-an-rgb-color
//
// Red:   y = (cos(x) + ((1 - cos(x)) / 3)) * 1.5
// Green: y = (1/3 * (1 - cos(x)) + sqrt(1/3) * sin(x)) * 1.5
// Blue:  y = (1/3 * (1 - cos(x)) - sqrt(1/3) * sin(x)) * 1.5

// Where y > 1 = 255
//   and y < 0 = 0

// RGB LED control
#include "Arduino.h"
#include "RGB.h"

// Constructor
RGB::RGB(int redPin, int greenPin, int bluePin, bool isCommonCathode = true) {
  _redPin = redPin;
  _greenPin = greenPin;
  _bluePin = bluePin;
  _isCommonCathode = isCommonCathode;
  
  _red = 0;
  _green = 0;
  _blue = 0;
}

// 
void RGB::setup() {
  // Set the RGB LEB pins as output
  pinMode(_redPin, OUTPUT);
  pinMode(_greenPin, OUTPUT);
  pinMode(_bluePin, OUTPUT);  
}


// Sets the color of the RGB LEB
void RGB::setColor(int red, int green, int blue) {
  int r = capValue(red);
  if (r != _red) {
    _red = r;
    analogWrite(_redPin, _red);
  }
  
  int g = capValue(green);
  if (g != _green) {
    _green = g;
    analogWrite(_greenPin, _green);
  }
  
  int b = capValue(blue);
  if (b != _blue) {
    _blue = b;
    analogWrite(_bluePin, _blue);  
  }
}


// where value is in the range 0 - 360
void RGB::cycleColor(int someValue, int brightness = 255) {
  
//  Serial.println(someValue);
  float x = (float) someValue * (3.1415926 / 180);
//  Serial.println(x);
  
  float br = (float) brightness;
  
  double r = (cos(x) + ((1.0 - cos(x)) / 3.0)) * 1.5 * br;
  double g = (1.0/3.0 * (1.0 - cos(x)) + sqrt(1.0/3.0) * sin(x)) * 1.5 * br;
  double b = (1.0/3.0 * (1.0 - cos(x)) - sqrt(1.0/3.0) * sin(x)) * 1.5 * br;
  
  setColor(r, g, b);
}


/***********
 * PRIVATE
 ***********/

// Caps a value to be with 0-255, and handles the common cathode vs
// anode switching
int RGB::capValue(int value) {
  if (value < 0) {
    value = 0;
  }
  if (value > 255) {
    value = 255;
  }
  if (_isCommonCathode) {
    return value;
  } else {
    return 255 - value;
  }
}