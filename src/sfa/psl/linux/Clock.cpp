#include <time.h>

#include "sfa/pal/Clock.hpp"

U64 Clock::nanoTime()
{
    timespec ts = {0, 0};
    (void) clock_gettime(CLOCK_REALTIME, &ts);
    return ((ts.tv_sec * NS_IN_S) + ts.tv_nsec);
}
