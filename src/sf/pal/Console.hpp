#ifndef SF_CONSOLE_HPP
#define SF_CONSOLE_HPP

#include "sf/core/BasicTypes.hpp"

namespace Console
{
    extern const char* const red;

    extern const char* const yellow;

    extern const char* const green;

    extern const char* const cyan;

    extern const char* const reset;

    I32 printf(const char* const kFmt, ...);
}

#endif
