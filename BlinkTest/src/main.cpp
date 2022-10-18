#define DEBUG 1
#include <my/debug.h>

#include <Arduino.h>
/*
 * Unblocking version of Blink
 */

bool on;
unsigned long myTime;

unsigned long interval = 200L;

bool lastIsHigh;
// int buttonPin = A5;
int buttonPin = LED_BUILTIN;
int buttonStatus = 0;

void setup()
{
    // put your setup code here, to run once:

    pinMode(buttonPin, OUTPUT);
    on = false;
    myTime = millis();

    /*
    lastIsHigh = false;
    digitalWrite(buttonPin, HIGH);

    */
}

void loop()
{
    // put your main code here, to run repeatedly:

    unsigned long currentTime = millis();
    if (currentTime - myTime >= interval || currentTime < myTime)
    {
        on = !on;
        myTime = currentTime;

        if (on)
        {
            digitalWrite(buttonPin, HIGH);
            DPL("Turn light on");
        }
        else
        {
            digitalWrite(buttonPin, LOW);
            DPL("Turn light off");
        }
    }

    /*
    int buttonValue = digitalRead(buttonPin);
    if (buttonValue == LOW) {
      if (lastIsHigh) {
        Serial.println("Get LOW");
        lastIsHigh = false;
      }
    } else {
      if (!lastIsHigh) {
        Serial.println("Get HIGH");
        lastIsHigh = true;
      }
    }
    */
}
