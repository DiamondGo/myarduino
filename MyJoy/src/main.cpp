#define LOG LEVEL_TRACE
#include <my/log.h>

#include "Arduino.h"
#include "Joystick.h"
#include "joybutton.h"
#include "ArduinoSTL.h"

using namespace std;

void setup()
{
#if LOG <= LEVEL_ERROR
    Serial.begin(4800);
#endif

    StickMachine::setupJoyStick();
}

void loop()
{
    long now = millis();

    vector<ButtonEvent> buttonEvents;

    processPinEvent(
        [&](Pin pin, SimpleEventType eventType)
        {
            TRACE("pin", getPinName(pin), "changed to state", eventType == Pushed ? "Pushed" : "Released", "at", now);
            JoyButton butt = Pin2Button(pin);
            // DEBUG("pin", pin, "to button", butt);
            ButtonState::get(butt)->handleEvent(now, eventType, buttonEvents);
        });

    if (buttonEvents.empty())
    {
        return;
    }

    StickMachine::handleEvents(buttonEvents);
}