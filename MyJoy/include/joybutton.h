#ifndef _JOYBUTTON_H_
#define _JOYBUTTON_H_

#include "pin2joy.h"

#include "ArduinoSTL.h"
using namespace std;

#define MS unsigned long
#define TIME_BIT_LEN 30
#define SHORT_TIME_BIT_LEN 26
#define AS_TIME(t) (t & (((MS)1 << TIME_BIT_LEN) - 1)) // keep only TIME_BIT_LEN bits
#define AS_SHORT_TIME(t) (t & (((MS)1 << SHORT_TIME_BIT_LEN) - 1))

class StickMachine; // forward declaration

struct ButtonEvent
{
    // use bit fields to reduce size from 8 to 4 bytes
    MS time : SHORT_TIME_BIT_LEN; // about 18 hours max, should be enough for game player?
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
    MS lastPressed : TIME_BIT_LEN;
    bool turbo : 1;
    // bool macro : 1;
    ButtonEvent *macro;
};

struct LastClick
{
    JoyButton button;
    MS time;
};

typedef int MKEY;
const MKEY NoMKEY = 0;
const MKEY M1 = 1;
const MKEY M2 = 1 << 1;
const MKEY M3 = M1 | M2;

class ButtonState
{
public:
    ButtonState(StickMachine *);
    virtual ~ButtonState();
    bool isHolding(JoyButton) const;
    MS getPressTime(JoyButton) const;
    void setPressTime(JoyButton, MS);
    bool isTurbo(JoyButton) const;
    void setTurbo(JoyButton, bool);
    bool hasMacro(JoyButton) const;
    // void setHasMacro(JoyButton, bool);
    void handleButtonEvent(MS timeInMS, JoyButton, SimpleEventType, vector<ButtonEvent> &);
    MKEY getModifyKey();

private:
    StickMachine *machine;
    ButtonInfo *buttons[MAX_BUTTON + 1];
    LastClick lastClick;
    const MS DOUBLE_CLICK_INTERVAL = 500;
};

enum WorkingState
{
    NORMAL,
    TURBO_HOLDING,
    RECORDING,
    REPLAYING,
};

const int AXIS_MAX = 127;
const int AXIS_MIN = -127;

class Recording
{
public:
    Recording(StickMachine *);
    virtual ~Recording();
    bool startRecording(JoyButton);
    bool isRecording();

private:
    const int MAX_RECORD_EVENT = 64;
    const int MAX_RECORD_START_IDLE_TIME = 5000; // in millisecond
    const int MAX_RECORD_END_IDLE_TIME = 5000;
    StickMachine *machine;
    JoyButton currentRecordButton;
};

class StickMachine
{
public:
    StickMachine();
    virtual ~StickMachine();
    void setupJoyStick();
    void handleEvents(vector<ButtonEvent> &);
    void handleOutput(vector<ButtonEvent> &);
    void handleDoubleClick(MS, JoyButton, vector<ButtonEvent> &);
    void handleModifiedClick(MS, MKEY, JoyButton, SimpleEventType);
    void handleTurbo(MS, vector<ButtonEvent> &);
    void toggleRecord(MS, JoyButton);
    void startRecording(MS, JoyButton);
    void stopRecording(MS);
    void handleReplay(MS, vector<ButtonEvent> &);
    ButtonState *getButtonState();
    void stateChangeSent(vector<ButtonEvent> &);
    void stateChangePress(JoyButton, SimpleEventType, MS);
    SimpleEventType getLastSentEvent(JoyButton);

    static StickMachine &getInstance();

private:
    Joystick_ *joystick;
    ButtonState *buttonState;
    Recording *recording;
    struct
    {
        MS lastTime : TIME_BIT_LEN;
        ;
        bool shouldPush : 1;
    } turboState;
    ButtonBinaryStatue lastSentIsPress;
    struct
    {
        WorkingState recordState;
        MS lastTime;
        JoyButton triggerButton;
        vector<ButtonEvent> macro;
    } state;

    void handleButton(JoyButton, SimpleEventType);
    void handleAxis(JoyButton, SimpleEventType);

    const MS TURBO_INTERVAL = 10;
    const MS TREMB_INTERVAL = 500;
    const MS MAX_COMBO_DURATION = 10 * 1000; // 10 seconds
    static StickMachine instance;
};

#endif