#include <cstdarg>
#include <cstdio>

#include "sf/pal/Console.hpp"

const char* const Console::red = "\033[0;31m";

const char* const Console::yellow = "\033[0;33m";

const char* const Console::green = "\033[0;32m";

const char* const Console::cyan = "\033[0;36m";

const char* const Console::reset = "\033[0m";

I32 Console::printf(const char* const kFmt, ...)
{
    va_list args;
    va_start(args, kFmt);
    const I32 ret = vprintf(kFmt, args);
    va_end(args);
    return ret;
}
