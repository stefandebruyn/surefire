#include "sfa/pal/Clock.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(Clock)
{
};

TEST(Clock, MonotonicNanoTime)
{
    U64 lastTimeNs = Clock::nanoTime();
    for (U32 i = 0; i < 100; ++i)
    {
        const U64 curTimeNs = Clock::nanoTime();
        CHECK_TRUE(curTimeNs > lastTimeNs);
        lastTimeNs = curTimeNs;
    }
}
