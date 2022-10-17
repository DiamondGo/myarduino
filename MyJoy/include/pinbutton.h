#ifndef PINBUTTON_H
#define PINBUTTON_H

#include "Arduino.h"
#include "function_objects.h"

typedef int Pin;

enum PinEventType {
    Pushed,     // connect
    Released    // disconnect
};

typedef struct {
    Pin pin;
    PinEventType type;
} PinEvent;


const Pin MAX_PIN = A5;

void setupPinButtons();

bool pinExist(Pin pin);
const char * getPinName(Pin pin);
int getPinState(Pin pin);
void setPinState(Pin pin, int vol);

typedef void (*EventHandle)(Pin, PinEventType);
void processPinEvent(EventHandle handle);
void processPinEvent(FunctionObject<void(Pin, PinEventType)> handle);

//typedef void (*printer_t)(int);
#endif