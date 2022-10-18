#ifndef __MY_DEBUG_H__
#define __MY_DEBUG_H__

#include <my/map.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#ifdef DEBUG // Macros are usually in all capital letters.
#if DEBUG

#define _SP(x) Serial.print(x)
#define _SPNL Serial.println()
#define _SPAPPEND(x)        \
    {                       \
        _SP(' ');           \
        _SP(x);             \
    }
#define _SPLOCATION     \
    {                   \
        _SP(__FILE__);  \
        _SP(':');       \
        _SP(__LINE__);  \
    }
#define _DP(...)                     \
    {                                \
        _SPLOCATION;                 \
        MAP(_SPAPPEND, __VA_ARGS__); \
    }
#define DP(...)             \
    {                       \
        _DP(__VA_ARGS__);   \
        _SPNL;              \
    }

#else
#define _DP(...) // now defines a blank line
#define DP(...)  // now defines a blank line
#endif
#else
#define _DP(...) // now defines a blank line
#define DP(...)  // now defines a blank line
#endif

#endif // __MY_DEBUG_H__
