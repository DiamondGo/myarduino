#ifndef _JOYBUTTON_H_
#define _JOYBUTTON_H_

#include "pin2joy.h"

#include "ArduinoSTL.h"
using namespace std;

#define TIME_BIT_LEN 30
#define SHORT_TIME_BIT_LEN 26
#define AS_TIME(t) (t & (((long)1 << TIME_BIT_LEN) - 1)) // keep only TIME_BIT_LEN bits
#define AS_SHORT_TIME(t) (t & (((long)1 << SHORT_TIME_BIT_LEN) - 1))

class StickMachine; // forward declaration

struct ButtonEvent
{
    // use bit fields to reduce size from 8 to 4 bytes
    long time : SHORT_TIME_BIT_LEN; // about 18 hours max, should be enough for game player?
    JoyButton button : 5;
    SimpleEventType event : 1;
};

struct ButtonBinaryStatue
{
public:
    ButtonBinaryStatue()
    {
        state = 0;
    }
    virtual ~ButtonBinaryStatue() {}
    void set(JoyButton jb, bool value)
    {
        if (value)
        {
            state |= (int32_t)1 << jb;
        }
        else
        {
            state &= ~((int32_t)1 << jb);
        }
    }
    bool get(JoyButton jb)
    {
        return ((state >> jb) & (int32_t)1) == (int32_t)1;
    }

private:
    int32_t state;
};

struct ButtonInfo
{
    long lastPressed : TIME_BIT_LEN;
    bool turbo : 1;
    bool macro : 1;
};

struct LastClick
{
    JoyButton button;
    long time;
};

typedef int MKEY;
const MKEY NoMKEY = 0;
const MKEY M1 = 1;
const MKEY M2 = 1 << 1;
const MKEY M3 = M1 & M2;

class ButtonState
{
public:
    ButtonState(StickMachine *);
    virtual ~ButtonState();
    bool isHolding(JoyButton) const;
    long getPressTime(JoyButton) const;
    void setPressTime(JoyButton, long);
    bool isTurbo(JoyButton) const;
    void setTurbo(JoyButton, bool);
    bool hasMacro(JoyButton) const;
    void setHasMacro(JoyButton, bool);
    void handleButtonEvent(long timeInMS, JoyButton, SimpleEventType, vector<ButtonEvent> &);
    MKEY getModifyKey();

private:
    StickMachine *machine;
    ButtonInfo *buttons[MAX_BUTTON + 1];
    LastClick lastClick;
    const long DOUBLE_CLICK_INTERVAL = 500;
};

enum StickState
{
    NORMAL,
    TURBO_HOLDING,
    RECORDING,
    REPLAYING,
};

const int AXIS_MAX = 127;
const int AXIS_MIN = -127;

class StickMachine
{
public:
    StickMachine();
    virtual ~StickMachine();
    void setupJoyStick();
    void handleEvents(vector<ButtonEvent> &);
    void handleOutput(vector<ButtonEvent> &);
    void handleDoubleClick(JoyButton, vector<ButtonEvent> &);
    void handleModifiedClick(MKEY, JoyButton);
    void handleTurbo(long, vector<ButtonEvent> &);
    ButtonState *getButtonState();
    void stateChangeSent(vector<ButtonEvent> &);
    void stateChangePress(JoyButton, SimpleEventType, long);
    SimpleEventType getLastSentEvent(JoyButton);

    static StickMachine &getInstance();

private:
    Joystick_ *joystick;
    ButtonState *buttonState;
    struct
    {
        long lastTime : TIME_BIT_LEN;
        ;
        bool shouldPush : 1;
    } turboState;
    ButtonBinaryStatue lastSentIsPress;

    void handleButton(JoyButton, SimpleEventType);
    void handleAxis(JoyButton, SimpleEventType);

    const long TURBO_INTERVAL = 5;
    static StickMachine instance;
};

#endif