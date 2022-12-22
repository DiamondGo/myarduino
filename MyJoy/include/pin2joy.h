#ifndef _PIN2JOY_H_
#define _PIN2JOY_H_

#include "pinbutton.h"
#include "Joystick.h" // https://github.com/MHeironimus/ArduinoJoystickLibrary

extern Joystick_ Joystick;
typedef SimpleEventType ButtonEventType;

enum JoyButton
{
    NONE = 0,
    Button1 = 6,
    Button2 = 7,
    Button3 = 11,
    Button4 = 4,
    Button5 = 5,
    Button6 = 10,
    Button7 = 13,
    Button8 = 12,
    UP = A5,
    DOWN = A4,
    LEFT = A3,
    RIGHT = A2,
    SELECT = 2,
    START = 3,
    FUNC1 = 8,
    FUNC2 = 9,
    MAX_BUTTON = MAX_PIN,
};

inline JoyButton Pin2Button(Pin pin)
{
    return JoyButton(pin);
}

static const JoyButton ButtonList[] = {
    Button1,
    Button2,
    Button3,
    Button4,
    Button5,
    Button6,
    Button7,
    Button8,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    START,
    FUNC1,
    FUNC2,
};

typedef void (*JoystickHandle)(JoyButton, ButtonEventType);

#endif