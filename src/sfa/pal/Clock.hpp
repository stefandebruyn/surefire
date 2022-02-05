#ifndef SFA_CLOCK_HPP
#define SFA_CLOCK_HPP

#include "sfa/core/BasicTypes.hpp"

namespace Clock
{
    constexpr U64 NS_IN_S = 1000000000;

    constexpr U64 US_IN_S = 1000000;

    constexpr U64 MS_IN_S = 1000;

    U64 nanoTime();

    inline void spinWait(const U64 kNs)
    {
        const U64 startNs = nanoTime();
        while ((nanoTime() - startNs) < kNs);
    }
}

#endif
