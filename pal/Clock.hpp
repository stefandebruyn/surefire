#ifndef SFA_CLOCK_HPP
#define SFA_CLOCK_HPP

#include "sfa/BasicTypes.hpp"

namespace Clock
{
    inline constexpr U64 NS_IN_S = 1000000000;

    inline constexpr U64 US_IN_S = 1000000;

    inline constexpr U64 MS_IN_S = 1000;

    U64 nanoTime();
}

#endif
