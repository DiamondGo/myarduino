#ifndef _JOYBUTTON_H_
#define _JOYBUTTON_H_

#include "pin2joy.h"

#include "ArduinoSTL.h"
using namespace std;

struct ButtonEvent
{
    long time;
    JoyButton button;
    SimpleEventType event;
};

typedef struct ButtonInfo
{
    long lastPressed;
} ButtonInfo;

class ButtonState
{
public:
    ButtonState();
    virtual ~ButtonState();
    bool isHolding(JoyButton) const;
    bool isTurbo(JoyButton) const;
    bool hasMacro(JoyButton) const;
    void handleEvent(long timeInMS, JoyButton, SimpleEventType, vector<ButtonEvent> &);

private:
    ButtonInfo *buttons[MAX_BUTTON + 1];
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
    ButtonState& buttonState();
    static StickMachine &getInstance();

private:
    Joystick_ *joystick;
    ButtonState buttons;

    void handleButton(JoyButton, SimpleEventType);
    void handleAxis(JoyButton, SimpleEventType);

    static StickMachine instance;
};

#endif