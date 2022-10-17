#include "utilities.h"

void pf(const char *format, ...)
{
#if defined(ENABLE_SERIAL_OUTPUT) && ENABLE_SERIAL_OUTPUT == true
    char buf[128];
    va_list ap;
    va_start(ap, format);
    vsprintf(buf, format, ap);
    Serial.println(buf);
#endif
}
