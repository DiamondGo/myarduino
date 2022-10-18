#include "Arduino.h"
#include "pinbutton.h"
#include "pin2joy.h"

static const char *PIN_NAME[] = {
    "", "",
    "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
    "",
    "A0", "A1", "A2", "A3", "A4", "A5"};
static int PIN_STATE[MAX_PIN + 1];

static bool setupOnce = false;

void setupPinButtons()
{
    if (!setupOnce)
    {
        for (Pin pin = 0; pin <= MAX_PIN; pin++)
        {
            if (pinExist(pin))
            {
                pinMode(pin, INPUT_PULLUP);
                setPinState(pin, HIGH);
            }
        }
        setupOnce = true;
    }
}

bool pinExist(Pin pin)
{
    return strlen(PIN_NAME[pin]) > 0;
}

const char *getPinName(Pin pin)
{
    return PIN_NAME[pin];
}

int getPinState(Pin pin)
{
    return PIN_STATE[pin];
}

void setPinState(Pin pin, int vol)
{
    PIN_STATE[pin] = vol;
}

void processPinEvent(EventHandle handle)
{
    processPinEvent([=](Pin pin, SimpleEventType eventType)
                    { handle(pin, eventType); });
}

void processPinEvent(FunctionObject<void(Pin, SimpleEventType)> handle)
{
    if (!setupOnce)
    {
        setupPinButtons();
    }

    for (JoyButton button : ButtonList)
    // for (Pin pin = 0; pin <= MAX_PIN; pin++)
    {
        auto pin = Pin(button);
        if (pinExist(pin))
        {
            int pinState = digitalRead(pin);
            if (pinState != getPinState(pin))
            {
                setPinState(pin, pinState);
                SimpleEventType event = (pinState == HIGH) ? Released : Pushed;
                handle(pin, event);
            }
        }
    }
}