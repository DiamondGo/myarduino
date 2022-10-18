#ifndef _PINBUTTON_H_
#define _PINBUTTON_H_

#include "Arduino.h"
#include "function_objects.h"

typedef int Pin;

enum SimpleEventType
{
    Pushed,  // connect
    Released // disconnect
};

typedef struct
{
    Pin pin;
    SimpleEventType type;
} PinEvent;

const Pin MAX_PIN = A5;

void setupPinButtons();

bool pinExist(Pin pin);
const char *getPinName(Pin pin);
int getPinState(Pin pin);
void setPinState(Pin pin, int vol);

typedef void (*EventHandle)(Pin, SimpleEventType);
void processPinEvent(EventHandle handle);
void processPinEvent(FunctionObject<void(Pin, SimpleEventType)> handle);

#endif