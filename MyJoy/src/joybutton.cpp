#define LOG LEVEL_DEBUG
#include "my/log.h"
#include "joybutton.h"

ButtonState::ButtonState(StickMachine *machine)
{
    this->machine = machine;
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

long ButtonState::getPressTime(JoyButton jb) const
{
    return buttons[jb]->lastPressed;
}

void ButtonState::setPressTime(JoyButton jb, long time)
{
    buttons[jb]->lastPressed = AS_TIME(time);
}

bool ButtonState::isTurbo(JoyButton jb) const
{
    return buttons[jb]->turbo;
}

bool ButtonState::hasMacro(JoyButton jb) const
{
    return buttons[jb]->macro;
}

void ButtonState::setTurbo(JoyButton jb, bool turbo)
{
    buttons[jb]->turbo = turbo;
}

void ButtonState::setHasMacro(JoyButton jb, bool hasMacro)
{
    buttons[jb]->macro = hasMacro;
}

void ButtonState::handleButtonEvent(long timeInMS, JoyButton jb, SimpleEventType event, vector<ButtonEvent> &output)
{
    this->machine->stateChangePress(jb, event, timeInMS);

    // Modify button
    if (jb != FUNC1 && jb != FUNC2)
    {
        auto mkey = this->getModifyKey();
        if (mkey != NoMKEY)
        {
            this->machine->handleModifiedClick(mkey, jb);
            // skip normal process
            // update botton state
            return;
        }
    }

    // Double click
    if (event == Released)
    {
        if (lastClick.button == jb && timeInMS - lastClick.time <= DOUBLE_CLICK_INTERVAL)
        {
            this->machine->handleDoubleClick(jb, output);
            lastClick.button = NONE;
            lastClick.time = 0;
        }
        else
        {
            lastClick.time = timeInMS;
            lastClick.button = jb;
        }
    }

    // only allow normal key
    if (jb == FUNC1 || jb == FUNC2)
    {
        /*
        this->lastSentIsPress.set(be.button, be.event == Pushed);
        this->buttonState->setPressTime(be.button, (be.event == Pushed) ? be.time : 0);
        */
        return;
    }

    if (this->machine->getLastSentEvent(jb) == event)
    {
        // ignore this, usually in turbo mode
        return;
    }
    // output
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
    this->buttonState = new ButtonState(this);
    this->turboState.shouldPush = false;
    this->turboState.lastTime = 0;
}

StickMachine::~StickMachine()
{
    delete this->joystick;
    delete this->buttonState;
}

void StickMachine::setupJoyStick()
{
    joystick->setXAxisRange(-127, 127);
    joystick->setYAxisRange(-127, 127);
    joystick->begin();
}

ButtonState *StickMachine::getButtonState()
{
    return this->buttonState;
}
void StickMachine::handleEvents(vector<ButtonEvent> &events)
{
    this->stateChangeSent(events);
    this->handleOutput(events);
}

void StickMachine::handleOutput(vector<ButtonEvent> &events)
{
    for (auto &buttonEvent : events)
    {
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
    }
    else // if event == Released
    {
        joystick->releaseButton(button);
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

void StickMachine::handleDoubleClick(JoyButton jb, vector<ButtonEvent> &output)
{
    switch (jb)
    {
    case FUNC1:
        DEBUG("Double click on Func1, clear all turbo buttons");
        for (auto jb : ButtonList)
        {
            this->buttonState->setTurbo(jb, false);
        }
        break;
    case FUNC2:
        DEBUG("Double click on Func2");
        break;
    default:
        break;
    }
}

MKEY ButtonState::getModifyKey()
{
    MKEY key = NoMKEY;
    if (this->buttons[FUNC1]->lastPressed > 0)
    {
        key |= M1;
    }

    if (this->buttons[FUNC2]->lastPressed > 0)
    {
        key |= M2;
    }

    return key;
}

void StickMachine::handleModifiedClick(MKEY mkey, JoyButton jb)
{
    DEBUG("MKEY", mkey, "with button", jb);

    switch (mkey)
    {
    case M1:
        // TURBO
        this->buttonState->setTurbo(jb, true);
        break;
    case M2:
        break;
    case M3:
        break;
    default:
        break;
    }
}

void StickMachine::handleTurbo(long time, vector<ButtonEvent> &output)
{
    if (time - this->turboState.lastTime < TURBO_INTERVAL)
    {
        TRACE("time - this->turboState.lastTime is", time - this->turboState.lastTime);
        return;
    }

    bool found = false;
    for (auto jb : ButtonList)
    {
        if (!this->buttonState->isTurbo(jb) || !this->buttonState->isHolding(jb))
        {
            continue;
        }

        found = true;
        if (this->turboState.shouldPush && !lastSentIsPress.get(jb))
        {
            // send press
            output.push_back(ButtonEvent{
                AS_SHORT_TIME(time),
                jb,
                Pushed});
            TRACE("Trigger turbo for button", jb, "as Pushed");
        }
        else if (!this->turboState.shouldPush && lastSentIsPress.get(jb))
        {
            // send release
            output.push_back(ButtonEvent{
                AS_SHORT_TIME(time),
                jb,
                Released});
            TRACE("Trigger turbo for button", jb, "as Released");
        }
    }

    this->turboState.shouldPush = !this->turboState.shouldPush;

    if (!found)
    {
        // set to default
        this->turboState.lastTime = time; // no loop in next interval
        this->turboState.shouldPush = false;
    }
}

void StickMachine::stateChangeSent(vector<ButtonEvent> &output)
{
    for (auto &be : output)
    {
        this->lastSentIsPress.set(be.button, be.event == Pushed);
    }
}

void StickMachine::stateChangePress(JoyButton jb, SimpleEventType event, long time)
{
    this->buttonState->setPressTime(jb, event == Pushed ? time : 0);
}

SimpleEventType StickMachine::getLastSentEvent(JoyButton jb)
{
    return this->lastSentIsPress.get(jb) ? Pushed : Released;
}