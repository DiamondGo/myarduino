#include "pin2joy.h"

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK, 32, 0,
                   true, true, false, false, false, false,
                   false, false, false, false, false);

static bool setupOnce = false;

void setupJoyStick() {
    if (!setupOnce)
    {
        Joystick.setXAxisRange(-127, 127);
        Joystick.setYAxisRange(-127, 127);
        Joystick.begin();

        setupOnce = true;
    }
}

void processButton() {
    
}