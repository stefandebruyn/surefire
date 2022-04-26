#include "sf/pal/Thread.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(Sbrio9637ThreadPriorityRange)
{
};

TEST(Sbrio9637ThreadPriorityRange, FairRange)
{
    CHECK_EQUAL(0, Thread::FAIR_MIN_PRI);
    CHECK_EQUAL(0, Thread::FAIR_MAX_PRI);
}

TEST(Sbrio9637ThreadPriorityRange, RealTimeRange)
{
    CHECK_EQUAL(2, Thread::REALTIME_MIN_PRI);
    CHECK_EQUAL(13, Thread::REALTIME_MAX_PRI);
}
