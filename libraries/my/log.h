#ifndef __MY_LOG_H__
#define __MY_LOG_H__

#include <my/map.h>

#define LEVEL_TRACE 1
#define LEVEL_DEBUG 2
#define LEVEL_INFO 3
#define LEVEL_WARN 4
#define LEVEL_ERROR 5
#define LEVEL_NONE 9

#ifndef LOG
#define LOG LEVEL_NONE
#endif

#ifdef LOG
#if LOG

#define _SP(x) Serial.print(x)
#define _SPNL Serial.println()
#define _SPAPPEND(x) \
    {                \
        _SP(' ');    \
        _SP(x);      \
    }
#define _SPLOCATION    \
    {                  \
        _SP(__FILE__); \
        _SP(':');      \
        _SP(__LINE__); \
    }
#define _DP(...)                     \
    {                                \
        _SPLOCATION;                 \
        MAP(_SPAPPEND, __VA_ARGS__); \
    }
#define DP(...)           \
    {                     \
        _DP(__VA_ARGS__); \
        _SPNL;            \
    }

#if LEVEL_TRACE >= LOG
#define TRACE(...)       \
    {                    \
        _SP("TRACE ");   \
        DP(__VA_ARGS__); \
    }
#else
#define TRACE(...)
#endif

#if LEVEL_DEBUG >= LOG
#define DEBUG(...)       \
    {                    \
        _SP("DEBUG ");   \
        DP(__VA_ARGS__); \
    }
#else
#define DEBUG(...)
#endif

#if LEVEL_INFO >= LOG
#define INFO(...)        \
    {                    \
        _SP("INFO  ");   \
        DP(__VA_ARGS__); \
    }
#else
#define INFO(...)
#endif

#if LEVEL_WARN >= LOG
#define WARN(...)        \
    {                    \
        _SP("WARN  ");   \
        DP(__VA_ARGS__); \
    }
#else
#define WARN(...)
#endif

#if LEVEL_ERROR >= LOG
#define ERROR(...)       \
    {                    \
        _SP("ERROR ");   \
        DP(__VA_ARGS__); \
    }
#else
#define ERROR(...)
#endif

#else
    #define _DP(...)
    #define DP(...)
    #define TRACE(...)
    #define DEBUG(...)
    #define INFO(...)
    #define WARN(...)
    #define ERROR(...)
#endif
#else
    #define _DP(...)
    #define DP(...)
    #define TRACE(...)
    #define DEBUG(...)
    #define INFO(...)
    #define WARN(...)
    #define ERROR(...)
#endif

#endif // __MY_LOG_H__
