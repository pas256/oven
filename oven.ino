/* Matthew's Microwave Oven
 */
#include <Wire.h> // Include the Arduino SPI library
#include "pitches.h"
#include "S7S.h"
#include "RotaryEncoder.h"
#include "RGB.h"

// Pin number the push button is connected to
const int buttonPin = 13;

// The push button states
int buttonState = 0;
int lastButtonState = 0;
boolean wasLongPress = false;

// This is the mode the program is currently in
int mode = 0;

// Number to display on the LED S7S
int displayValue = 0;
int lastDisplayValue = 0;

// Pin numbers for the RGB LEB
const int ledRedPin = 6;
const int ledGreenPin = 9;
const int ledBluePin = 10;
RGB rgb(ledRedPin, ledGreenPin, ledBluePin, false);

// I2C address of our S7S
const byte s7sAddress = 0x71;
S7S s7s(s7sAddress);

// Will be used with sprintf to create strings
char tempString[10];  

// Piezo buzzer pin number
int buzzerPin = 7;

// Place to store current time
unsigned long now;
long offset = 0;
  
// The time (in ms) when the button was pressed down
unsigned long buttonDownTime;
boolean cycleColors = true;

// Amount of time (in ms) to hold the button down to set the clock
const int buttonSetClockDuration = 5000;  // 10 seconds


const int MODE_CLOCK = 0;
const int MODE_SET_COOK_TIME = 1;
const int MODE_COOK = 2;
const int MODE_SET_CLOCK = 3;



void setup() {
  Serial.begin(115200);
  
  // Attach interrupts for rotary encoder
  re_setup();
    
  // Enable push button in LOW state
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, LOW);

  // Set the RGB LEB pins as output
  rgb.setup();
  
  // Initialize hardware I2C pins for S7S
  Wire.begin();

  // Clear display, reset cursor
  s7s.clearDisplay();

  // Set baud rate higher
  s7s.setBaudRate(6);
  
  // Set brightness
  s7s.setBrightness(255);

  // Welcome
  s7s.sendString("-HI-");
  Serial.println("Hi");
  delay(2000);

  // Clear the display before jumping into loop
  s7s.clearDisplay();
}


void loop() {
  doButtonCheck();
  
  switch (mode) {
    case MODE_CLOCK:
      displayClock();
      break;
    case MODE_SET_COOK_TIME:
      adjustCountdownTimer();
      break;
    case MODE_COOK:
      cooking();
      break;
    case MODE_SET_CLOCK:
      setClock();
      break;
  }  
}


void doButtonCheck() {
  // Read the state of the button
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    if (lastButtonState == HIGH) {
      Serial.println("Button Pushed");
      switchMode();
    }
  } else {
    if (lastButtonState == LOW) {
      // Record time button was pushed
      buttonDownTime = millis();
    }

    // Check for long press
    if (millis() - buttonDownTime >= buttonSetClockDuration) {
        wasLongPress = true;

        // Set time setup goes here
        rgb.setColor(0, 128, 255);
        cycleColors = false;
        
        //offset = (pulses / 4) * 1000 * 60;
        long pulses = offset / 60 / 1000 * 4;
        re_setPulses(pulses);
    }
  }
  
  // Save the previous state for the next loop
  lastButtonState = buttonState;
}


void switchMode() {
  if (wasLongPress) {
    mode = MODE_SET_CLOCK;
    wasLongPress = false;
    
  } else {
    // Normal mode switch
    mode++;
    mode = mode % 3;
    Serial.print("Now in mode: "); 
    Serial.println(mode);
    
    // Reset pulses for normal mode switch
    re_resetPulses();
    
    cycleColors = true;
  }
  
  // Always clear display before switching modes
  s7s.clearDisplay();
}


void displayClock() {
  now = millis() + offset + 86400000;
  int seconds = (now / 1000) % 60;
  int minutes = (now / 1000 / 60) % 60;
  int hours24 = (now / 1000 / 60 / 60) % 24;
  int hours = hours24 % 12;
    
  if (hours == 0) {
    hours = 12;
  }
  
  sprintf(tempString, "%2d%02d", hours, minutes);
  s7s.sendString(tempString);
  
  if (mode == MODE_SET_CLOCK) {
    // Decimals
    if (hours24 >= 12) {
      s7s.setDecimals(0b00011000); 
    } else {
      s7s.setDecimals(0b00010000); 
    }
  } else {
    // Decimals
    if (seconds % 2 == 0) {
      if (hours24 >= 12) {
        s7s.setDecimals(0b00011000); 
      } else {
        s7s.setDecimals(0b00010000); 
      }
    } else {
      if (hours24 >= 12) {
        s7s.setDecimals(0b00001000); 
      } else {
        s7s.setDecimals(0b00000000);
      }
    }
    
    // Brightness
    int s7sBrightness = 255;
    int buttonBrightness = 255;
    if (hours24 < 7 || hours24 >= 20) {
      s7sBrightness = 32;
      buttonBrightness = 32;
    }
    s7s.setBrightness(s7sBrightness);

    // Cycle colors
    if (cycleColors) {
      int v = (now / 100) % 360;
      rgb.cycleColor(v, buttonBrightness);
    }
  }  
}


void setClock() {
  rgb.setColor(0, 0, 255);

  long pulses = (long) re_getPulses();
  offset = (pulses / 4) * 1000 * 60;

  displayClock();
}


void adjustCountdownTimer() {
  rgb.setColor(0, 255, 0);

  s7s.setBrightness(255);

  int pulses = re_getPulses();
  if (pulses < 0) {
    re_resetPulses();
  }
  displayValue = pulses / 4;
  updateDisplay();
}


void cooking() {
  rgb.setColor(255, 0, 0);

  s7s.setBrightness(255);

  displayValue--;
  updateDisplay();
  
  if (displayValue <= 0) {
    soupsReady();
  } else {
    for (int i = 0; i < 5; i++) {
      tone(buzzerPin, NOTE_C1, 100);
      delay(100);
      tone(buzzerPin, NOTE_C2, 100);
      delay(100);
    }
    noTone(buzzerPin);
  }
}


void soupsReady() {
  // Switch to clock mode after song and dance
  mode = MODE_CLOCK;
  
  displayValue = 0;  
  re_resetPulses();

  // notes in the melody:
  int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4 };

  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  int noteDurations[] = {
    8, 8, 4, 4, 4, 2 };

  int colors[][3] = {
    {255, 255, 0},
    {255, 255, 255}, // was 128 Blue (pin 10)
    {255, 255, 0},
    {255, 128, 0},
    {255, 0, 0},
    {255, 0, 255}
  };

  char* display[] = {
    "   0",
    "   0",
    "  00",
    " 000",
    "0000",
    "0  0"
  };
  
  int numberOfNotes = sizeof(melody) / sizeof(int);
  
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < numberOfNotes; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1800/noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);

    // Change LED color
    rgb.setColor(colors[thisNote][0], colors[thisNote][1], colors[thisNote][2]);
    s7s.sendString(display[thisNote]);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  } 
}

void updateDisplay() {
  // Magical sprintf creates a string for us to send to the s7s.
  //  The %4d option creates a 4-digit integer.
  sprintf(tempString, "%4d", displayValue);

  // This will output the tempString to the S7S
  s7s.sendString(tempString);
}






