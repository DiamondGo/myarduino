#define LOG LEVEL_DEBUG
#include "my/log.h"
#include "joybutton.h"
#include <EEPROM.h>

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
    return buttons[jb]->macro != nullptr;
}

ButtonEvent *ButtonState::getMacro(JoyButton jb) const
{
    return buttons[jb]->macro;
}

void ButtonState::setMacro(JoyButton jb, vector<ButtonEvent> &macro)
{

    ButtonInfo *info = buttons[jb];
    if (info->macro != nullptr)
    {
        delete[] info->macro;
        info->macro = nullptr;
    }

    if (!macro.empty())
    {
        info->macro = new ButtonEvent[macro.size() + 1];
        MS start = macro[0].time;
        DEBUG("start is", start);
        ButtonEvent *p = info->macro;
        ButtonEvent *previous = nullptr;
        for (auto it = macro.rbegin(); it != macro.rend(); it++) // in reverse order
        {
            DEBUG("saved macro: button", it->button, "event", it->event, "time", it->time - start);
            if (previous != nullptr)
            {
                if (previous->time == AS_SHORT_TIME(it->time - start) && previous->button == it->button && previous->event == it->event)
                {
                    // skip this
                    continue;
                }
            }

            p->time = AS_SHORT_TIME(it->time - start);
            p->button = it->button;
            p->event = it->event;
            previous = p;
            p++;
        }
        p->button = NONE; // serves as tail null
        TRACE("record", macro.size(), "events for button", jb);
    }
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
    this->turboState.changedTime = 0;

    this->state.recordState = NORMAL;
    this->state.changedTime = 0;
    this->state.triggerButton = NONE;
    this->state.mirror = false;
    this->state.speedUp = DEFAULT_SPEEDUP;
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
        TRACE("Double click on Func1, clear all turbo buttons");
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
        TRACE("Double click on Func2");
        this->state.mirror = !this->state.mirror;
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
    TRACE("MKEY", mkey, "with button", jb);

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
        if (event == Released && this->state.recordState == NORMAL)
        {
            switch (jb)
            {
            case START:
                DEBUG("M2 + START")
                // reset macro state
                this->state.speedUp = DEFAULT_SPEEDUP;
                this->state.mirror = false;
                break;
            case SELECT:
                DEBUG("M2 + SELECT")
                this->state.speedUp = float(this->state.speedUp / 2.0);
                break;
            default:
                break;
            }
        }
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

    if (time - this->turboState.changedTime < TURBO_INTERVAL)
    {
        TRACE("time - this->turboState.lastTime is", time - this->turboState.changedTime);
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
        this->turboState.changedTime = time; // no loop in next interval
        this->turboState.shouldPush = false;
    }
}

void StickMachine::toggleRecord(MS now, JoyButton jb)
{
    if (now - this->state.changedTime < TREMBLE_INTERVAL)
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
    this->state.changedTime = now;
    this->state.macro.clear();
    this->state.triggerButton = jb;
    TRACE("start recording for button", jb);
}

void StickMachine::stopRecording(MS now)
{
    if (this->state.recordState != RECORDING)
    {
        return;
    }

    // record macro to button
    if (!this->state.macro.empty() || (now - this->state.changedTime > TREMBLE_INTERVAL && now - this->state.changedTime < CANCEL_INTERVAL))
    {
        this->buttonState->setMacro(this->state.triggerButton, this->state.macro);
    }

    this->state.recordState = NORMAL;
    this->state.changedTime = now;
    this->state.macro.clear();
    TRACE("stop recording for button", this->state.triggerButton);
    this->state.triggerButton = NONE;

    this->saveState();
}

void StickMachine::handleMacro(MS time, vector<ButtonEvent> &output)
{
    if (this->state.recordState == RECORDING)
    {
        // quit recording for overtime
        if (output.empty() && time - this->state.changedTime >= MAX_COMBO_DURATION)
        {
            this->stopRecording(time);
            return;
        }

        for (auto be : output)
        {
            this->state.macro.push_back(be);
        }
    }

    else if (this->state.recordState == REPLAYING)
    {
        this->continueReplay(time, output);
        return;
    }

    else if (this->state.recordState == NORMAL)
    {
        // check if button with macro is released
        for (auto it = output.begin(); it != output.end(); it++)
        {
            if (it->event == Released && this->buttonState->hasMacro(it->button))
            {
                this->startReplay(time, it->button, output);
                it = output.erase(it);
                break;
            }
        }
    }
}

void StickMachine::startReplay(MS now, JoyButton jb, vector<ButtonEvent> &output)
{
    this->state.changedTime = now;
    this->state.triggerButton = jb;
    this->state.recordState = REPLAYING;
    this->state.macro.clear();
    auto macro = this->buttonState->getMacro(jb);
    while (macro != nullptr && macro->button != NONE)
    {
        ButtonEvent be = *macro;
        if (this->state.mirror)
        {
            if (be.button == LEFT)
            {
                be.button = RIGHT;
            }
            else if (be.button == RIGHT)
            {
                be.button = LEFT;
            }
        }
        be.time = (MS)(be.time * this->state.speedUp);
        DEBUG("macro button", be.button, "time", be.time, "event is", be.event);
        this->state.macro.push_back(be);
        macro++;
    }

    DEBUG("record contains", this->state.macro.size(), "events");
    this->continueReplay(now, output);
}

void StickMachine::continueReplay(MS now, vector<ButtonEvent> &output)
{
    MS start = this->state.changedTime;
    auto &macro = this->state.macro;
    while (!macro.empty() && macro.back().time <= now - start)
    {
        auto be = macro.back();
        be.time = now;
        DEBUG("replay button", be.button, "event", be.event);
        output.push_back(be);
        macro.pop_back();
    }

    if (macro.empty())
    {
        this->state.recordState = NORMAL;
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

void writeEEP(int pos, void *data, int size)
{
    for (int i = 0; i < size; i++)
    {
        EEPROM.write(pos + i, *((uint8_t *)data + i));
    }
}

void readEEP(int pos, void *data, int size)
{
    for (int i = 0; i < size; i++)
    {
        *((uint8_t *)data + i) = EEPROM.read(pos + i);
    }
}

void StickMachine::loadState()
{
    DEBUG("in loadState");
    int eeplen = EEPROM.length();
    int headpos = 0;
    int tailpos = eeplen;

    EEPHeader h;
    readEEP(headpos, &h, sizeof(h));
    headpos += sizeof(h);

    for (int i = 0; i < h.macroCount; i++)
    {
        EEPMacroHeader mh;
        readEEP(headpos, &mh, sizeof(mh));
        headpos += sizeof(mh);
        DEBUG("load macro for button", mh.button, "location is", mh.location);

        vector<ButtonEvent> macro;
        // for (int j = 0; int(mh.location + j * sizeof(EEPButtonEvent)) < tailpos; j++)
        for (int j = 1; tailpos - j * sizeof(EEPButtonEvent) >= mh.location; j++)
        {
            EEPButtonEvent ebe;
            readEEP(tailpos - j * sizeof(EEPButtonEvent), &ebe, sizeof(ebe));
            ButtonEvent be;
            be.button = ebe.button;
            be.time = ebe.time;
            be.event = ebe.pressed ? Pushed : Released;
            macro.push_back(be);
            DEBUG("read event for button", be.button, "time is", be.time, "event type is", be.event);
        }
        tailpos = mh.location;

        this->buttonState->setMacro(mh.button, macro);
    }

    INFO("load macro for", h.macroCount, "buttons");
}

void StickMachine::saveState()
{
    int eeplen = EEPROM.length();
    int headpos = 0;
    int tailpos = eeplen;

    // get macro count
    int macroCount = 0;
    for (auto jb : ButtonList)
    {
        if (this->getButtonState()->hasMacro(jb))
        {
            macroCount++;
        }
    }

    EEPHeader h;
    h.macroCount = macroCount;
    writeEEP(headpos, &h, sizeof(h));
    headpos += sizeof(h);

    for (auto jb : ButtonList)
    {
        if (this->getButtonState()->hasMacro(jb))
        {
            DEBUG("save macro for button", jb)
            EEPMacroHeader mh;
            mh.button = jb;

            ButtonEvent *macro = this->getButtonState()->getMacro(jb);
            int beCount = 0;
            while ((macro + beCount) != nullptr && (macro + beCount)->button != NONE)
            {
                beCount++;
            }
            mh.location = tailpos - beCount * sizeof(EEPButtonEvent);
            DEBUG("event count is", beCount, "location is", mh.location, "/", tailpos);
            tailpos = mh.location;

            // write macro header
            writeEEP(headpos, &mh, sizeof(mh));
            headpos += sizeof(mh);

            for (int i = 0; i < beCount; i++)
            {
                ButtonEvent &be = macro[i];
                EEPButtonEvent ebe;
                ebe.button = be.button;
                ebe.pressed = (be.event == Pushed);
                ebe.time = be.time;
                writeEEP(tailpos + i * sizeof(EEPButtonEvent), &ebe, sizeof(ebe));
                DEBUG("write event to eeprom, button", ebe.button, ", event", ebe.pressed, ", time", ebe.time);
            }
        }
    }

    INFO("save", macroCount, "macros to eeprom");
}
