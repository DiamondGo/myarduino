#ifndef __MY_DEBUG_H__
#define __MY_DEBUG_H__

#ifndef DEBUG
#define DEBUG 1
#endif

#ifdef DEBUG  // Macros are usually in all capital letters.
    #if DEBUG
    #define DPL(...)                     \
        {                                \
            Serial.print(__FILE__);      \
            Serial.print(':');           \
            Serial.print(__LINE__);      \
            Serial.print(' ');           \
            Serial.println(__VA_ARGS__); \
        } // DPRINT is a macro, debug print
    #else
        #define DPL(...) // now defines a blank line
    #endif
#else
    #define DPL(...) // now defines a blank line
#endif

#endif // __MY_DEBUG_H__

