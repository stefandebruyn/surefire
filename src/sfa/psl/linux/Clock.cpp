#include <time.h>

#include "sfa/pal/Clock.hpp"

/// @note The return value of `clock_gettime` is disregarded because no errors
///       should be possible. `EFAULT` cannot occur since the `timespec` pointer
///       is trivially valid. `EINVAL` cannot occur since `CLOCK_REALTIME`
///       should only be defined on systems with a real-time clock available.
///
/// @note If by some chance `clock_gettime` does fail, a time of 0 is returned.
///
/// @note `CLOCK_REALTIME` is used so that times reflect adjustments made by
///       protocols like NTP. This means that time may not be monotonic if
///       the real-time clock is adjusted between calls to `nanoTime`. Care
///       should also be taken that the system does not automatically adjust the
///       real-time clock, e.g., for Daylight Saving Time.
U64 Clock::nanoTime()
{
    timespec ts = {0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((ts.tv_sec * NS_IN_S) + ts.tv_nsec);
}
