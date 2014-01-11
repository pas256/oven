// RGB LED

#ifndef RGB_LED_H
#define RGB_LED_H

#include "Arduino.h"

class RGB {
  public:
    RGB(int redPin, int greenPin, int bluePin, bool isCommonCathode);
//    ~RGB();

    void setup();
    
    void setColor(int red, int green, int blue);
    
    void cycleColor(int someValue, int brightness);
    
  private:
    int _redPin;
    int _greenPin;
    int _bluePin;
    bool _isCommonCathode;

    int _red;
    int _green;
    int _blue;
    
    int capValue(int value);
};

#endif