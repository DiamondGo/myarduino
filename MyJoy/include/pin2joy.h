#ifndef PIN2JOY_H
#define PIN2JOY_H

#include "pinbutton.h"
#include "Joystick.h" // https://github.com/MHeironimus/ArduinoJoystickLibrary

extern Joystick_ Joystick;
typedef PinEventType ButtonEventType;

enum JoyPin {
    Button1 = 2,
    Button2,
    Button3,
    Button4,
    Button5,
    Button6,
    Button7,
    Button8,
    Button9,
    Button10,
    Button11,
    Button12,
    UP = A0,
    DOWN = A1,
    LEFT = A2,
    RIGHT = A3,
};

void setupJoyStick();
typedef void (*JoystickHandle)(JoyPin, ButtonEventType);
void processButton();
#endif