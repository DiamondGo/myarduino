#ifndef UTILITIES_H
#define UTILITIES_H

#include "Arduino.h"

#ifndef ENABLE_SERIAL_OUTPUT
#define ENABLE_SERIAL_OUTPUT true
#endif

void pf(const char *format, ...);

#endif