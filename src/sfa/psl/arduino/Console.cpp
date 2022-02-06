#include <Arduino.h>
#include <stdarg.h>

#include "sfa/pal/Console.hpp"

static constexpr U32 gPrintBufSize = 128;

I32 Console::printf(const char* const kFmt, ...)
{
    static char buf[gPrintBufSize];
    va_list args;
    va_start(args, kFmt);
    const I32 ret = vsnprintf(buf, sizeof(buf), kFmt, args);
    Serial.print(buf);
    va_end(args);
    return ret;
}
