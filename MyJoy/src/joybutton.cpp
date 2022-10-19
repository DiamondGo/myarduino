#define LOG LEVEL_ERROR
#include "my/log.h"
#include "joybutton.h"


ButtonState::ButtonState()
{
    for (auto jb : ButtonList)
    {
        buttons[jb] = new ButtonInfo();
    }
}

ButtonState::~ButtonState()
{
    for (auto jb : ButtonList)
    {
        delete buttons[jb];
    }
}

bool ButtonState::isHolding(JoyButton jb) const
{
    return buttons[jb]->lastPressed > 0;
}

bool ButtonState::isTurbo(JoyButton jb) const
{
    return false;
}

bool ButtonState::hasMacro(JoyButton jb) const
{
    return false;
}

void ButtonState::handleEvent(long timeInMS, JoyButton jb, SimpleEventType event, vector<ButtonEvent> &output)
{
    DEBUG("ButtonState handleEvent:", this->button);
    output.push_back(
        ButtonEvent{
            timeInMS,
            jb,
            event});
}

StickMachine StickMachine::instance;
StickMachine &StickMachine::getInstance()
{
    return instance;
}

StickMachine::StickMachine()
{
    this->joystick = new Joystick_(JOYSTICK_DEFAULT_REPORT_ID,
                                   JOYSTICK_TYPE_JOYSTICK, 32, 0,
                                   true, true, false, false, false, false,
                                   false, false, false, false, false);
}

StickMachine::~StickMachine()
{
    delete this->joystick;
}

void StickMachine::setupJoyStick()
{
    joystick->setXAxisRange(-127, 127);
    joystick->setYAxisRange(-127, 127);
    joystick->begin();
}

ButtonState& StickMachine::buttonState()
{
    return this->buttons;
}

void StickMachine::handleEvents(vector<ButtonEvent> &events)
{
    for (auto &buttonEvent : events)
    {
        DEBUG("process JoyButton", buttonEvent.button);
        switch (buttonEvent.button)
        {
        case Button1:
        case Button2:
        case Button3:
        case Button4:
        case Button5:
        case Button6:
        case Button7:
        case Button8:
        case START:
        case SELECT:
            handleButton(buttonEvent.button, buttonEvent.event);
            break;
        case UP:
        case DOWN:
        case LEFT:
        case RIGHT:
            handleAxis(buttonEvent.button, buttonEvent.event);
            break;

        default:
            break;
        }
    }
}

void StickMachine::handleButton(JoyButton jb, SimpleEventType event)
{
    uint8_t button;
    switch (jb)
    {
    case Button1:
        button = 0;
        break;
    case Button2:
        button = 1;
        break;
    case Button3:
        button = 2;
        break;
    case Button4:
        button = 3;
        break;
    case Button5:
        button = 4;
        break;
    case Button6:
        button = 5;
        break;
    case Button7:
        button = 6;
        break;
    case Button8:
        button = 7;
        break;

    case START:
        button = 8;
        break;
    case SELECT:
        button = 9;
        break;
    default:
        break;
    }

    if (event == Pushed)
    {
        joystick->pressButton(button);
        DEBUG("Button", button, "pressed");
    }
    else // if event == Released
    {
        joystick->releaseButton(button);
        DEBUG("Button", button, "released");
    }
}

void StickMachine::handleAxis(JoyButton jb, SimpleEventType event)
{
    switch (jb)
    {
    case UP:
        joystick->setYAxis(event == Pushed ? AXIS_MIN : 0);
        break;
    case DOWN:
        joystick->setYAxis(event == Pushed ? AXIS_MAX : 0);
        break;
    case LEFT:
        joystick->setXAxis(event == Pushed ? AXIS_MIN : 0);
        break;
    case RIGHT:
        joystick->setXAxis(event == Pushed ? AXIS_MAX : 0);
        break;
    default:
        break;
    }
}