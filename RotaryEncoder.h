// Rotary Encoder class

#ifndef RotaryEncoder_H
#define RotaryEncoder_H

#include "Arduino.h"

// Performs the necessary interrupt setup
void re_setup();

// Returns the number of pulses counted so far
int re_getPulses();

// Manually set the number of pulses
void re_setPulses(int number);

// Short-cut method to set pulses to zero
void re_resetPulses();

void re_aRise();

void re_aFall();

void re_bRise();

void re_bFall();

#endif