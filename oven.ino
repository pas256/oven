/* Matthew's Microwave Oven
 */
#include <Wire.h> // Include the Arduino SPI library

// Pin number the push button is connected to
const int buttonPin = 13;

// The last state the button was in
int buttonState = 0;
int lastButtonState = 0;

// This MOD the number of states determines the state we are in
int buttonPressCounter = 0;

// Potentiometer PIN
const int potentiometerPin = A0;

// Value of the potentiometer
int potentiometerValue = 0;
int lastPotentiometerValue = 0;

// Pin numbers for the RGB LEB
const int ledRedPin = 9;
const int ledGreenPin = 10;
const int ledBluePin = 11;

// I2C address of our S7S
const byte s7sAddress = 0x71;



unsigned int counter = 990;  // This variable will count up to 65k
char tempString[10];  // Will be used with sprintf to create strings

void setup()
{
  Serial.begin(9600);
  
  // Enable push button in LOW state
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, LOW);

  // Set the RGB LEB pins as output
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);  
   
  Wire.begin();  // Initialize hardware I2C pins

  // Clear the display, and then turn on all segments and decimals
  clearDisplayI2C();  // Clears display, resets cursor

  // Custom function to send four bytes via I2C
  //  The I2C.write function only allows sending of a single
  //  byte at a time.
  s7sSendStringI2C("-HI-");
  delay(2000);

  // Clear the display before jumping into loop
  clearDisplayI2C();  
}

void loop()
{
  // Read value of potentiometer
  potentiometerValue = analogRead(potentiometerPin);
  if (potentiometerValue != lastPotentiometerValue) {
    Serial.println(potentiometerValue); 
  }
  lastPotentiometerValue = potentiometerValue;

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
      setColor(255, 0, 0);
      countDown();
      break;
    case 2:
      setColor(0, 0, 0);
      clearDisplayI2C();
      counter = 0;
      break;
  }  
}


void adjustTime() {
  counter = potentiometerValue;
  printCounter();
}

void countDown() {
  counter--;
  printCounter();
  
  if (counter == 0) {
    soupsReady();
  } else {
    delay(500);
  }
}

void soupsReady() {
  buttonPressCounter = 2;
  setColor(255, 255, 0);
  s7sSendStringI2C("   0");
  delay(500);

  setColor(0, 255, 255);
  s7sSendStringI2C("  00");
  delay(500);

  setColor(255, 0, 255);
  s7sSendStringI2C(" 000");
  delay(500);
  
  setColor(255, 255, 255);
  s7sSendStringI2C("0000");
  delay(500);
}

void printCounter() {
  // Magical sprintf creates a string for us to send to the s7s.
  //  The %4d option creates a 4-digit integer.
  sprintf(tempString, "%4d", counter);

  // This will output the tempString to the S7S
  s7sSendStringI2C(tempString);
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

// This custom function works somewhat like a serial.print.
//  You can send it an array of chars (string) and it'll print
//  the first 4 characters in the array.
void s7sSendStringI2C(String toSend)
{
  Wire.beginTransmission(s7sAddress);
  for (int i=0; i<4; i++)
  {
    Wire.write(toSend[i]);
  }
  Wire.endTransmission();
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplayI2C()
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightnessI2C(byte value)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x7A);  // Set brightness command byte
  Wire.write(value);  // brightness data byte
  Wire.endTransmission();
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimalsI2C(byte decimals)
{
  Wire.beginTransmission(s7sAddress);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission();
}
