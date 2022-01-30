#include <cstdio>
#include <cstdarg>

#include "pal/Console.hpp"

I32 Console::printf(const char* const kFmt, ...)
{
    va_list args;
    va_start(args, kFmt);
    const I32 ret = vprintf(kFmt, args);
    va_end(args);
    return ret;
}
