#define LOG LEVEL_DEBUG
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

    StickMachine::getInstance().setupJoyStick();
}

void loop()
{
    long now = AS_TIME(millis());

    vector<ButtonEvent> buttonEvents;

    processPinEvent(
        [&](Pin pin, SimpleEventType eventType)
        {
            TRACE("pin", getPinName(pin), "changed to state", eventType == Pushed ? "Pushed" : "Released", "at", now);
            JoyButton butt = Pin2Button(pin);
            StickMachine::getInstance().getButtonState()->handleButtonEvent(now, butt, eventType, buttonEvents);
        });

    // search for hold button in turbo mode
    StickMachine::getInstance().handleTurbo(now, buttonEvents);

    if (buttonEvents.empty())
    {
        return;
    }

    StickMachine::getInstance().handleEvents(buttonEvents);
}