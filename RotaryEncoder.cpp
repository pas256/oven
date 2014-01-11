// Rotary Encoder class

#include "Arduino.h"
#include "RotaryEncoder.h"

// Debugging flag
const bool DEBUGGING = false;

// Pulse counter
long _pulses = 0;

// Rotary encoder signals
int _A_SIG = 0, _B_SIG = 0;


// Performs the necessary interrupt setup
void re_setup() {
  attachInterrupt(0, re_aRise, RISING);
  attachInterrupt(1, re_bRise, RISING);
}

// Returns the number of pulses counted so far
long re_getPulses() {
  return _pulses;
}

// Manually set the number of pulses
void re_setPulses(int number) {
  _pulses = number;
}

// Short-cut method to set pulses to zero
void re_resetPulses() {
  _pulses = 0;
}


void re_aRise(){
 detachInterrupt(0);
 _A_SIG = 1;
 
 if (_B_SIG == 0) {
   _pulses--; //moving forward
 }
 if (_B_SIG == 1) {
   _pulses++; //moving reverse
 }
 
 if (DEBUGGING) Serial.println(_pulses);
 attachInterrupt(0, re_aFall, FALLING);
}

void re_aFall(){
  detachInterrupt(0);
 _A_SIG = 0;
 
 if (_B_SIG == 1) {
   _pulses--; //moving forward
 }
 if (_B_SIG == 0) {
   _pulses++; //moving reverse
 }
 
 if (DEBUGGING) Serial.println(_pulses);
 attachInterrupt(0, re_aRise, RISING);  
}

void re_bRise(){
 detachInterrupt(1);
 _B_SIG = 1;
 
 if (_A_SIG == 1) {
   _pulses--; //moving forward
 }
 if (_A_SIG == 0) {
   _pulses++; //moving reverse
 }
 
 if (DEBUGGING) Serial.println(_pulses);
 attachInterrupt(1, re_bFall, FALLING);
}

void re_bFall(){
 detachInterrupt(1);
 _B_SIG = 0;
 
 if (_A_SIG == 0) {
   _pulses--; //moving forward
 }
 if (_A_SIG == 1) {
   _pulses++; //moving reverse
 }

 if (DEBUGGING) Serial.println(_pulses);
 attachInterrupt(1, re_bRise, RISING);
}
