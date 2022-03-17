#ifndef SF_CLOCK_HPP
#define SF_CLOCK_HPP

#include "sf/core/BasicTypes.hpp"

namespace Clock
{
    constexpr U64 NS_IN_S = 1000000000;

    constexpr U64 US_IN_S = 1000000;

    constexpr U64 MS_IN_S = 1000;

    /// @brief Value reserved by the framework to represent the absence of a
    /// time value. This is the largest time value possible at ~584 years, so it
    /// should never be produced by the clock.
    constexpr U64 NO_TIME = 0xFFFFFFFFFFFFFFFF;

    U64 nanoTime();

    inline void spinWait(const U64 kNs)
    {
        const U64 startNs = nanoTime();
        while ((nanoTime() - startNs) < kNs);
    }
}

#endif
