#include <Arduino.h>

/*
 * Unblocking version of Blink
 */

bool on;
unsigned long myTime;

unsigned long interval = 2000L;

bool lastIsHigh;
int buttonPin = A5;
int buttonStatus = 0;

void setup()
{
    // put your setup code here, to run once:

    // pinMode(LED_BUILTIN, OUTPUT);
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
            // digitalWrite(LED_BUILTIN, HIGH);
            digitalWrite(buttonPin, HIGH);
            Serial.println("Turn light on");
        }
        else
        {
            // digitalWrite(LED_BUILTIN, LOW);
            digitalWrite(buttonPin, LOW);
            Serial.println("Turn light off");
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
