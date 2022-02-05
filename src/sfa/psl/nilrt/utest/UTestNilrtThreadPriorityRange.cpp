#include "sfa/pal/Thread.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(NilrtThreadPriorityRange)
{
};

TEST(NilrtThreadPriorityRange, FairRange)
{
    CHECK_EQUAL(0, Thread::FAIR_MIN_PRI);
    CHECK_EQUAL(0, Thread::FAIR_MAX_PRI);
}

TEST(NilrtThreadPriorityRange, RealTimeRange)
{
    CHECK_EQUAL(2, Thread::REALTIME_MIN_PRI);
    CHECK_EQUAL(13, Thread::REALTIME_MAX_PRI);
}
