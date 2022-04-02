#ifndef SF_STATE_MACHINE_COMMAND_HPP
#define SF_STATE_MACHINE_COMMAND_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/core/BasicTypes.hpp"

namespace Cli
{
    I32 sm(const Vec<String> kArgs);

    I32 smCheck(const Vec<String> kArgs);

    I32 smTest(const Vec<String> kArgs);

    I32 smAutocode(const Vec<String> kArgs);
}

#endif
