/**
 * 
 */
#include "Arduino.h"
#include "Joystick.h"
#include "utilities.h"
#include "pinbutton.h"

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 32, 0,
  true, true, false, false, false, false,
  false, false, false, false, false);

void setup()
{
    Serial.begin(4800);
}

void loop()
{
    long now = millis();
    
    processPinEvent([=](Pin pin, PinEventType eventType){
        pf("pin %s changed to state %s at %ld", getPinName(pin), eventType == Pushed ? "Pushed" : "Released", now);
    });
}