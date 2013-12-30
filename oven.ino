/* Matthew's Microwave Oven
 */
#include <Wire.h> // Include the Arduino SPI library
#include "pitches.h"
#include "S7S.h"
#include "RotaryEncoder.h"

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
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);  
  
  // Initialize hardware I2C pins for S7S
  Wire.begin();

  // Clear display, reset cursor
  s7s.clearDisplay();

  // Welcome
  s7s.sendString("-HI-");
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
        Serial.println("Button pressed for 10 seconds");
        wasLongPress = true;

        // Set time setup goes here
        setColor(0, 128, 255);
        re_resetPulses();
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
  }
}


void displayClock() {
  setColor(0, 0, 0);
  
  now = millis() + offset + 86400000;
  int seconds = (now / 1000) % 60;
  int minutes = (now / 1000 / 60) % 60;
  int hours = (now / 1000 / 60 / 60) % 12;
  
  if (hours == 0) {
    hours = 12;
  }
  
  sprintf(tempString, "%02d%02d", hours, minutes);
  s7s.sendString(tempString);
  
  if (seconds % 2 == 0) {
    s7s.setDecimals(0b00010000); 
  } else {
    s7s.setDecimals(0b00000000);
  }
}


void setClock() {
  setColor(0, 0, 255);

  long pulses = (long) re_getPulses();
  offset = (pulses / 4) * 1000 * 60;

  displayClock();
  s7s.setDecimals(0b00010000);
}


void adjustCountdownTimer() {
  setColor(0, 255, 0);

  int pulses = re_getPulses();
  if (pulses < 0) {
    re_resetPulses();
  }
  displayValue = pulses / 4;
  updateDisplay();
}


void cooking() {
  setColor(255, 0, 0);

  displayValue--;
  updateDisplay();
  
  if (displayValue <= 0) {
    soupsReady();
  } else {
    for (int i = 0; i < 5; i++) {
      tone(buzzerPin, NOTE_C1, 50);
      delay(50);
      tone(buzzerPin, NOTE_C2, 50);
      delay(50);
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
    setColor(colors[thisNote][0], colors[thisNote][1], colors[thisNote][2]);
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



// Sets the color of the RGB LEB
void setColor(int red, int green, int blue)
{
  analogWrite(ledRedPin, 255 - red);
  analogWrite(ledGreenPin, 255 - green);
  analogWrite(ledBluePin, 255 - blue);  
}


