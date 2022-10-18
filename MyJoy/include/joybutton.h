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

class ButtonState
{
public:
    ButtonState(JoyButton);
    bool isHolding() const;
    bool isTurbo() const;
    bool hasMacro() const;
    void handleEvent(long timeInMS, SimpleEventType, vector<ButtonEvent>&);

    static void init();
    static ButtonState *get(JoyButton);

private:
    JoyButton button;
    long downTime;

    static ButtonState *states[];
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
    static void setupJoyStick();
    static void handleEvents(vector<ButtonEvent> &);
private:
    static Joystick_ joystick;
    static void handleButton(JoyButton, SimpleEventType);
    static void handleAxis(JoyButton, SimpleEventType);
};

#endif