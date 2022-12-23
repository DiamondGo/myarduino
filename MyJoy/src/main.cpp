#define LOG LEVEL_INFO
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
    StickMachine::getInstance().loadState();
}

void loop()
{
    auto now = AS_TIME(millis());

    /*
    static MS loadTs = 0;
    if (loadTs == 0)
    {
        loadTs = now;
    }
    if (now - loadTs >= 5000)
    {
        StickMachine::getInstance().loadState();
        loadTs = now;
    }
    */

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

    // handle replay
    StickMachine::getInstance().handleMacro(now, buttonEvents);

    // handle interpreted events
    StickMachine::getInstance().handleEvents(buttonEvents);
}