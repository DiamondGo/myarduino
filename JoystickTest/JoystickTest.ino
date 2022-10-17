#include <Joystick.h>

// Create the Joystick
Joystick_ Joystick;

// Constant that maps the physical pin to the joystick button.
const int pinToButtonMap = 16;

void setup() {
  // put your setup code here, to run once:
  
  // Initialize Button Pins
  pinMode(pinToButtonMap, INPUT_PULLUP);

  // Initialize Joystick Library
  Joystick.begin();

  Serial.begin(9600);
}

// Last state of the button
int lastButtonState = 0;

void loop() {
  char buf[40];
  
  // put your main code here, to run repeatedly:
  // Read pin values
  int currentButtonState = !digitalRead(pinToButtonMap);
  if (currentButtonState != lastButtonState)
  {
  Joystick.setButton(0, currentButtonState);
  lastButtonState = currentButtonState;
  sprintf(buf, "Pin %d", pinToButtonMap);
  Serial.println(buf);
  }

  delay(50);
}
