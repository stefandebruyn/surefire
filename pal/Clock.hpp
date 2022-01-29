#ifndef SFA_CLOCK_HPP
#define SFA_CLOCK_HPP

#include "sfa/BasicTypes.hpp"

namespace Clock
{
    static constexpr U64 NS_IN_S = 1000000000ULL;

    U64 nanoTime();
}

#endif
