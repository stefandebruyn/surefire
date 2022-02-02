#include "UTestThreadCommon.hpp"

TEST_GROUP(ThreadFair)
{
    void setup()
    {
        threadTestSetup();
    }

    void teardown()
    {
        threadTestTeardown();
    }
};

TEST(ThreadFair, PriorityRange)
{
    CHECK_TRUE(Thread::FAIR_MIN_PRI <= Thread::FAIR_MAX_PRI);

    for (I32 i = Thread::FAIR_MIN_PRI; i <= Thread::FAIR_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flag,
                                     i,
                                     Thread::FAIR,
                                     Thread::ALL_CORES,
                                     gThreads[0]));
        Result threadRes = -1;
        CHECK_SUCCESS(Thread::await(gThreads[0], &threadRes));
        CHECK_SUCCESS(threadRes);
        CHECK_TRUE(flag);
    }
}

TEST(ThreadFair, PriorityTooLow)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(noop,
                               nullptr,
                               (Thread::FAIR_MIN_PRI - 1),
                               Thread::FAIR,
                               Thread::ALL_CORES,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}

TEST(ThreadFair, PriorityTooHigh)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(noop,
                               nullptr,
                               (Thread::FAIR_MAX_PRI + 1),
                               Thread::FAIR,
                               Thread::ALL_CORES,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}
