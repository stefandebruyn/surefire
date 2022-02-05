#include "sfa/pal/Thread.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(LinuxThreadPriorityRange)
{
};

TEST(LinuxThreadPriorityRange, FairRange)
{
    CHECK_EQUAL(0, Thread::FAIR_MIN_PRI);
    CHECK_EQUAL(0, Thread::FAIR_MAX_PRI);
}

TEST(LinuxThreadPriorityRange, RealTimeRange)
{
    CHECK_EQUAL(1, Thread::REALTIME_MIN_PRI);
    CHECK_EQUAL(99, Thread::REALTIME_MAX_PRI);
}
