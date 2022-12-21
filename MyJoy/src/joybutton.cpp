#define LOG LEVEL_DEBUG
#include "my/log.h"
#include "joybutton.h"

ButtonInfo *newButtonInfo()
{
    auto b = new ButtonInfo();
    b->lastPressed = 0;
    b->turbo = false;
    b->macro = nullptr;
    return b;
}

ButtonState::ButtonState(StickMachine *machine)
{
    this->machine = machine;
    for (auto jb : ButtonList)
    {
        buttons[jb] = newButtonInfo();
    }
}

ButtonState::~ButtonState()
{
    for (auto jb : ButtonList)
    {
        delete[] buttons[jb]->macro;
        delete buttons[jb];
    }
}

bool ButtonState::isHolding(JoyButton jb) const
{
    return buttons[jb]->lastPressed > 0;
}

MS ButtonState::getPressTime(JoyButton jb) const
{
    return buttons[jb]->lastPressed;
}

void ButtonState::setPressTime(JoyButton jb, MS time)
{
    buttons[jb]->lastPressed = AS_TIME(time);
}

bool ButtonState::isTurbo(JoyButton jb) const
{
    return buttons[jb]->turbo;
}

bool ButtonState::hasMacro(JoyButton jb) const
{
    return buttons[jb]->macro == nullptr;
}

void ButtonState::setTurbo(JoyButton jb, bool turbo)
{
    buttons[jb]->turbo = turbo;
}

void ButtonState::handleButtonEvent(MS timeInMS, JoyButton jb, SimpleEventType event, vector<ButtonEvent> &output)
{
    this->machine->stateChangePress(jb, event, timeInMS);

    // Modify button
    if (jb != FUNC1 && jb != FUNC2)
    {
        auto mkey = this->getModifyKey();
        if (mkey != NoMKEY)
        {
            this->machine->handleModifiedClick(timeInMS, mkey, jb, event);
            // skip normal process
            // update button state
            return;
        }
    }

    // Double click
    if (event == Released)
    {
        if (lastClick.button == jb && timeInMS - lastClick.time <= DOUBLE_CLICK_INTERVAL)
        {
            this->machine->handleDoubleClick(timeInMS, jb, output);
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
    TRACE("Button", jb, "triggered", (event == Pushed ? "Pushed" : "Released"), "by user");
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
    this->recording = new Recording(this);

    this->turboState.shouldPush = false;
    this->turboState.lastTime = 0;

    this->state.recordState = NORMAL;
    this->state.lastTime = 0;
    this->state.triggerButton = NONE;
}

StickMachine::~StickMachine()
{
    delete this->joystick;
    delete this->buttonState;
    delete this->recording;
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

void StickMachine::handleDoubleClick(MS time, JoyButton jb, vector<ButtonEvent> &output)
{
    switch (jb)
    {
    case FUNC1:
        DEBUG("Double click on Func1, clear all turbo buttons");
        for (auto b : ButtonList)
        {
            if (this->buttonState->isTurbo(b))
            {
                this->buttonState->setTurbo(b, false);
                if (!this->buttonState->isHolding(b) && this->lastSentIsPress.get(b))
                {
                    output.push_back(ButtonEvent{
                        time,
                        b,
                        Released});
                }
            }
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

void StickMachine::handleModifiedClick(MS now, MKEY mkey, JoyButton jb, SimpleEventType event)
{
    DEBUG("MKEY", mkey, "with button", jb);

    switch (mkey)
    {
    case M1:
        // TURBO
        if (event == Released && this->state.recordState == NORMAL)
        {
            this->buttonState->setTurbo(jb, true);
        }
        break;
    case M2:
        break;
    case M3:
        // Record / Replay
        if (event == Released &&
            (this->state.recordState == NORMAL || this->state.recordState == RECORDING))
        {
            this->toggleRecord(now, jb);
        }
        break;
    default:
        break;
    }
}

void StickMachine::handleTurbo(MS time, vector<ButtonEvent> &output)
{
    // don't handle turbo in other modes
    if (this->state.recordState != NORMAL)
    {
        return;
    }

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
            TRACE("Button", jb, "triggered as Pushed by turbo");
        }
        else if (!this->turboState.shouldPush && lastSentIsPress.get(jb))
        {
            // send release
            output.push_back(ButtonEvent{
                AS_SHORT_TIME(time),
                jb,
                Released});
            TRACE("Button", jb, "triggered as Released by turbo");
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

void StickMachine::toggleRecord(MS now, JoyButton jb)
{
    if (now - this->state.lastTime < TREMB_INTERVAL)
    {
        return;
    }

    switch (this->state.recordState)
    {
    case NORMAL:
        this->startRecording(now, jb);
        break;
    case RECORDING:
        this->stopRecording(now);
        break;
    default:
        break;
    }
}

void StickMachine::startRecording(MS now, JoyButton jb)
{
    if (this->state.recordState != NORMAL)
    {
        return;
    }
    this->state.recordState = RECORDING;
    this->state.lastTime = now;
    this->state.macro.clear();
    this->state.triggerButton = jb;
    DEBUG("start recording for button", jb);
}

void StickMachine::stopRecording(MS now)
{
    if (this->state.recordState != RECORDING)
    {
        return;
    }
    this->state.recordState = NORMAL;
    this->state.lastTime = now;
    // TODO: record macro to button
    this->state.macro.clear();
    DEBUG("stop recording for button", this->state.triggerButton);
    this->state.triggerButton = NONE;
}

void StickMachine::handleReplay(MS time, vector<ButtonEvent> &output)
{
    if (this->state.recordState == RECORDING)
    {
        if (output.size() == 0 && time - this->state.lastTime >= MAX_COMBO_DURATION)
        {
            this->stopRecording(time);
        }
    }

    if (this->state.recordState == REPLAYING)
    {
        return;
    }
}

void StickMachine::stateChangeSent(vector<ButtonEvent> &output)
{
    for (auto &be : output)
    {
        this->lastSentIsPress.set(be.button, be.event == Pushed);
    }
}

void StickMachine::stateChangePress(JoyButton jb, SimpleEventType event, MS time)
{
    this->buttonState->setPressTime(jb, event == Pushed ? time : 0);
}

SimpleEventType StickMachine::getLastSentEvent(JoyButton jb)
{
    return this->lastSentIsPress.get(jb) ? Pushed : Released;
}

Recording::Recording(StickMachine *machine)
{
    this->machine = machine;
}

Recording::~Recording()
{
}