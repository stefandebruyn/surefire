#include "pal/Clock.hpp"
#include "UTestThreadCommon.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

struct ThreadArgs final
{
    bool flag;
    U64 tReturnNs;
    U64 waitNs;
};

static ThreadArgs gArgs1;
static ThreadArgs gArgs2;
static ThreadArgs gArgs3;

/////////////////////////////////// Helpers ////////////////////////////////////

static Result spinOnFlagAndRecordTime(void* kArgs)
{
    ThreadArgs* const args = (ThreadArgs*) kArgs;
    while (args->flag == false);
    args->tReturnNs = Clock::nanoTime();
    return SUCCESS;
}

static Result spinAndRecordTime(void* kArgs)
{
    ThreadArgs* const args = (ThreadArgs*) kArgs;
    Clock::spinWait(args->waitNs);
    args->tReturnNs = Clock::nanoTime();
    return SUCCESS;
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(ThreadRealTime)
{
    void setup()
    {
        // Clear global thread args.
        gArgs1 = {};
        gArgs2 = {};
        gArgs3 = {};
    }

    void teardown()
    {
        threadTestTeardown();
    }
};

/// Real-time threads can be successfully created at every valid priority.
TEST(ThreadRealTime, PriorityRange)
{
    CHECK_TRUE(Thread::REALTIME_MIN_PRI <= Thread::REALTIME_MAX_PRI);

    for (I32 i = Thread::REALTIME_MIN_PRI; i <= Thread::REALTIME_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flag,
                                     i,
                                     Thread::REALTIME,
                                     Thread::ALL_CORES,
                                     gTestThreads[0]));
        Result threadRes = -1;
        CHECK_SUCCESS(gTestThreads[0].await(&threadRes));
        CHECK_SUCCESS(threadRes);
        CHECK_TRUE(flag);
    }
}

TEST(ThreadRealTime, PriorityTooLow)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(nop,
                               nullptr,
                               (Thread::REALTIME_MIN_PRI - 1),
                               Thread::REALTIME,
                               Thread::ALL_CORES,
                               gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}

TEST(ThreadRealTime, PriorityTooHigh)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(nop,
                               nullptr,
                               (Thread::REALTIME_MAX_PRI + 1),
                               Thread::REALTIME,
                               Thread::ALL_CORES,
                               gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}

/// @note This test assumes that a larger priority value corresponds to higher
///       priority.
TEST(ThreadRealTime, RealTimeSameAffinity)
{
    // Threads 2 and 3 will spin for 250 ms before returning.
    gArgs2.waitNs = (0.25 * Clock::NS_IN_S);
    gArgs3.waitNs = gArgs2.waitNs;

    // Create 3 real-time threads with descending priorities on the same core.
    // The first thread blocks the other 2 by spinwaiting until we set a flag.
    // Threads will record the time of their return in the argument structs
    // passed to them.
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs1,
                                 (Thread::REALTIME_MIN_PRI + 2),
                                 Thread::REALTIME,
                                 0,
                                 gTestThreads[0]));
    CHECK_SUCCESS(Thread::create(spinAndRecordTime,
                                 &gArgs2,
                                 (Thread::REALTIME_MIN_PRI + 1),
                                 Thread::REALTIME,
                                 0,
                                 gTestThreads[1]));
    CHECK_SUCCESS(Thread::create(spinAndRecordTime,
                                 &gArgs3,
                                 Thread::REALTIME_MIN_PRI,
                                 Thread::REALTIME,
                                 0,
                                 gTestThreads[2]));

    // Wait for a relatively long time to avoid racing.
    Clock::spinWait(0.25 * Clock::NS_IN_S);

    // At this point no threads have run, so all return times are unset.
    CHECK_EQUAL(0, gArgs1.tReturnNs);
    CHECK_EQUAL(0, gArgs2.tReturnNs);
    CHECK_EQUAL(0, gArgs3.tReturnNs);

    // Release first thread from its spin.
    gArgs1.flag = true;

    // Wait for threads in expected order of completion.
    CHECK_SUCCESS(gTestThreads[0].await(nullptr));
    CHECK_SUCCESS(gTestThreads[1].await(nullptr));
    CHECK_SUCCESS(gTestThreads[2].await(nullptr));

    // Threads ran in the order of their priorities.
    CHECK_TRUE(gArgs1.tReturnNs < gArgs2.tReturnNs);
    CHECK_TRUE(gArgs2.tReturnNs < gArgs3.tReturnNs);

    // Time elapsed between each thread returning is at least the time spent
    // spinning by the last two threads.
    CHECK_TRUE((gArgs2.tReturnNs - gArgs1.tReturnNs) >= gArgs2.waitNs);
    CHECK_TRUE((gArgs3.tReturnNs - gArgs2.tReturnNs) >= gArgs3.waitNs);
}

/// @note This test requires that affinities 0 and 1 be valid on the current
///       platform.
TEST(ThreadRealTime, RealTimeDifferentAffinity)
{
    // Create 2 real-time threads with different priorities on different cores.
    // Each thread spinwaits on a different flag.
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs1,
                                 Thread::REALTIME_MIN_PRI,
                                 Thread::REALTIME,
                                 0,
                                 gTestThreads[0]));
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs2,
                                 Thread::REALTIME_MAX_PRI,
                                 Thread::REALTIME,
                                 1,
                                 gTestThreads[1]));

    // Wait for a relatively long time to avoid racing.
    Clock::spinWait(0.25 * Clock::NS_IN_S);

    // At this point no threads have run, so all return times are unset.
    CHECK_EQUAL(0, gArgs1.tReturnNs);
    CHECK_EQUAL(0, gArgs2.tReturnNs);

    // Release lower priority thread from its spin and wait for it to complete.
    // This succeeds because the other thread, though still spinning and higher
    // priority, is on a different core.
    gArgs1.flag = true;
    CHECK_SUCCESS(gTestThreads[0].await(nullptr));

    // At this point only the lower priority thread has set its return time.
    CHECK_TRUE(gArgs1.tReturnNs != 0);
    CHECK_EQUAL(0, gArgs2.tReturnNs);

    // Release and wait on higher priority thread.
    gArgs2.flag = true;
    CHECK_SUCCESS(gTestThreads[1].await(nullptr));
}
