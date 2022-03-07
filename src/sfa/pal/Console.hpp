#ifndef SFA_CONSOLE_HPP
#define SFA_CONSOLE_HPP

#include "sfa/core/BasicTypes.hpp"

namespace Console
{
    extern const char* const red;

    extern const char* const green;

    extern const char* const cyan;

    extern const char* const reset;

    I32 printf(const char* const kFmt, ...);
}

#endif
