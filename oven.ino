/* Matthew's Microwave Oven
 */
#include <Wire.h> // Include the Arduino SPI library
#include "pitches.h"
#include "S7S.h"

// Pin number the push button is connected to
const int buttonPin = 13;

// The last state the button was in
int buttonState = 0;
int lastButtonState = 0;

// This MOD the number of states determines the state we are in
int buttonPressCounter = 0;

// Number to display on the LED S7S
int displayValue = 0;
int lastDisplayValue = 0;

// Rotary encoder signals
int pulses, A_SIG=0, B_SIG=1;

// Pin numbers for the RGB LEB
const int ledRedPin = 8;
const int ledGreenPin = 9;
const int ledBluePin = 10;

// I2C address of our S7S
const byte s7sAddress = 0x71;
S7S s7s(s7sAddress);

// Will be used with sprintf to create strings
char tempString[10];  

int buzzerPin = 7;

void setup()
{
  Serial.begin(9600);
  
  // Attach interrupts for rotary encoder
  attachInterrupt(0, A_RISE, RISING);
  attachInterrupt(1, B_RISE, RISING);
    
  // Enable push button in LOW state
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, LOW);

  // Set the RGB LEB pins as output
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);  
   
  Wire.begin();  // Initialize hardware I2C pins

  // Clear the display, and then turn on all segments and decimals
  s7s.clearDisplay();  // Clears display, resets cursor

  // Custom function to send four bytes via I2C
  //  The I2C.write function only allows sending of a single
  //  byte at a time.
  s7s.sendString("-HI-");
  delay(2000);

  // Clear the display before jumping into loop
  s7s.clearDisplay();  
}

void loop()
{
  if (hasButtonBeenPushed()) {
    Serial.println("Button Pushed"); 
    buttonPressCounter++;
    Serial.println(buttonPressCounter); 
  }
  
  switch (buttonPressCounter % 3) {
    case 0:
      setColor(0, 255, 0);
      adjustTime();
      break;
    case 1:
      cooking();
      break;
    case 2:
      setColor(0, 0, 0);
      s7s.clearDisplay();
      displayValue = 0;
      pulses = 0;
      break;
  }  
}


void adjustTime() {
  if (pulses < 0) {
    pulses = 0;
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
      tone(buzzerPin, NOTE_D1, 50);
      delay(50);
    }
  }
}

void soupsReady() {
  buttonPressCounter = 2;

  // notes in the melody:
  int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4 };

  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  int noteDurations[] = {
    8, 8, 4, 4, 4, 2 };

  int colors[][3] = {
    {255, 255, 0},
    {255, 255, 128},
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
    Serial.println(melody[thisNote]);
    Serial.println(noteDuration);
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

/* Interrupt emulation. Detect when the button was pushed once and only once
 * while it is held down
 */ 
boolean hasButtonBeenPushed() {
  boolean hasButtonBeenPushed = false;
  
  // Read the state of the button
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
    if (lastButtonState == HIGH) {
      hasButtonBeenPushed = true;
    }
  }
  lastButtonState = buttonState;
  
  return hasButtonBeenPushed;
}


// Sets the color of the RGB LEB
void setColor(int red, int green, int blue)
{
  analogWrite(ledRedPin, 255 - red);
  analogWrite(ledGreenPin, 255 - green);
  analogWrite(ledBluePin, 255 - blue);  
}

void A_RISE(){
 detachInterrupt(0);
 A_SIG=1;
 
 if(B_SIG==0)
 pulses++;//moving forward
 if(B_SIG==1)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(0, A_FALL, FALLING);
}

void A_FALL(){
  detachInterrupt(0);
 A_SIG=0;
 
 if(B_SIG==1)
 pulses++;//moving forward
 if(B_SIG==0)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(0, A_RISE, RISING);  
}

void B_RISE(){
 detachInterrupt(1);
 B_SIG=1;
 
 if(A_SIG==1)
 pulses++;//moving forward
 if(A_SIG==0)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(1, B_FALL, FALLING);
}

void B_FALL(){
 detachInterrupt(1);
 B_SIG=0;
 
 if(A_SIG==0)
 pulses++;//moving forward
 if(A_SIG==1)
 pulses--;//moving reverse
 Serial.println(pulses);
 attachInterrupt(1, B_RISE, RISING);
}
