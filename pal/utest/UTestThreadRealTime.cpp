#include "pal/Thread.hpp"
#include "pal/Clock.hpp"
#include "UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

inline constexpr U32 gThreadsSize = 16;

static I32 gThreads[gThreadsSize];

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

static Result noop(void* kArgs)
{
    return SUCCESS;
}

static Result setFlag(void* kArgs)
{
    *((bool*) kArgs) = true;
    return SUCCESS;
}

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
        // Check that `gThreads` array is large enough to store the maximum
        // number of thread descriptors. This is necessary since the array
        // cannot be statically sized according to `Thread::MAX_THREADS`, and
        // we want to avoid allocating memory in this test to keep it portable.
        // If this check fails, increase `gThreadsSize`.
        CHECK_TRUE(gThreadsSize >= Thread::MAX_THREADS);

        // Reset global thread descriptors.
        for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
        {
            gThreads[i] = -1;
        }

        // Clear global thread args.
        gArgs1 = {};
        gArgs2 = {};
        gArgs3 = {};
    }

    void teardown()
    {
        // Attempt to wait on all threads in case the test failed with threads
        // still alive. If the test passed, these waits fail silently.
        for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
        {
            Thread::await(gThreads[i], nullptr);
        }
    }
};

TEST(ThreadRealTime, CreateMaxThreads)
{
    // Array of flags to be set by threads.
    bool flags[Thread::MAX_THREADS + 1] = {};

    // Create max number of threads.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flags[i],
                                     Thread::REALTIME_MIN_PRI,
                                     Thread::REALTIME,
                                     Thread::NO_AFFINITY,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(setFlag,
                               &flags[0],
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               Thread::NO_AFFINITY,
                               gThreads[0]));

    // Wait for all threads to finish.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        Result threadRes = -1;
        CHECK_SUCCESS(Thread::await(gThreads[i], &threadRes));
        CHECK_SUCCESS(threadRes);
        // Flag was set by thread.
        CHECK_TRUE(flags[i]);
    }

    // Clear flags.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        flags[i] = false;
    }

    // Create max number of threads again to ensure the interface is reusable.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flags[i],
                                     Thread::REALTIME_MIN_PRI,
                                     Thread::REALTIME,
                                     Thread::NO_AFFINITY,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(setFlag,
                               &flags[0],
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               Thread::NO_AFFINITY,
                               gThreads[0]));

    // Wait for all threads to finish.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        Result threadRes = -1;
        CHECK_SUCCESS(Thread::await(gThreads[i], &threadRes));
        CHECK_SUCCESS(threadRes);
        // Flag was set by thread.
        CHECK_TRUE(flags[i]);
    }
}

TEST(ThreadRealTime, PriorityRange)
{
    for (I32 i = Thread::REALTIME_MIN_PRI; i <= Thread::REALTIME_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flag,
                                     i,
                                     Thread::REALTIME,
                                     Thread::NO_AFFINITY,
                                     gThreads[0]));
        Result threadRes = -1;
        CHECK_SUCCESS(Thread::await(gThreads[0], &threadRes));
        CHECK_SUCCESS(threadRes);
        CHECK_TRUE(flag);
    }
}

TEST(ThreadRealTime, PriorityTooLow)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(noop,
                               nullptr,
                               (Thread::REALTIME_MIN_PRI - 1),
                               Thread::REALTIME,
                               Thread::NO_AFFINITY,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}

TEST(ThreadRealTime, PriorityTooHigh)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::create(noop,
                               nullptr,
                               (Thread::REALTIME_MAX_PRI + 1),
                               Thread::REALTIME,
                               Thread::NO_AFFINITY,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}

// @note This test assumes that a larger priority value corresponds to higher
//       priority.
TEST(ThreadRealTime, RealTimeSameAffinity)
{
    // Threads 2 and 3 will spin for 250 ms before returning.
    gArgs2.waitNs = (0.25 * Clock::NS_IN_S);
    gArgs3.waitNs = gArgs2.waitNs;

    // Create 3 real-time threads with descending priorities on the same core.
    // The first thread blocks the other 2 by spin-waiting until we set a flag.
    // Threads will record the time of their return in the argument structs
    // passed to them.
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs1,
                                 (Thread::REALTIME_MIN_PRI + 2),
                                 Thread::REALTIME,
                                 0,
                                 gThreads[0]));
    CHECK_SUCCESS(Thread::create(spinAndRecordTime,
                                 &gArgs2,
                                 (Thread::REALTIME_MIN_PRI + 1),
                                 Thread::REALTIME,
                                 0,
                                 gThreads[1]));
    CHECK_SUCCESS(Thread::create(spinAndRecordTime,
                                 &gArgs3,
                                 Thread::REALTIME_MIN_PRI,
                                 Thread::REALTIME,
                                 0,
                                 gThreads[2]));

    // Wait for a relatively long time to avoid racing.
    Clock::spinWait(0.25 * Clock::NS_IN_S);

    // At this point no threads have run, so all return times are unset.
    CHECK_EQUAL(0, gArgs1.tReturnNs);
    CHECK_EQUAL(0, gArgs2.tReturnNs);
    CHECK_EQUAL(0, gArgs3.tReturnNs);

    // Release first thread from its spin.
    gArgs1.flag = true;

    // Wait for threads in expected order of completion.
    CHECK_SUCCESS(Thread::await(gThreads[0], nullptr));
    CHECK_SUCCESS(Thread::await(gThreads[1], nullptr));
    CHECK_SUCCESS(Thread::await(gThreads[2], nullptr));

    // Threads ran in the order of their priorities.
    CHECK_TRUE(gArgs1.tReturnNs < gArgs2.tReturnNs);
    CHECK_TRUE(gArgs2.tReturnNs < gArgs3.tReturnNs);

    // Time elapsed between each thread returning is at least the time spent
    // spinning by the last two threads.
    CHECK_TRUE((gArgs2.tReturnNs - gArgs1.tReturnNs) >= gArgs2.waitNs);
    CHECK_TRUE((gArgs3.tReturnNs - gArgs2.tReturnNs) >= gArgs3.waitNs);
}

// @note This test requires that affinities 0 and 1 be valid on the current
//       platform.
TEST(ThreadRealTime, RealTimeDifferentAffinity)
{
    // Create 2 real-time threads with different priorities on different cores.
    // Each thread spin-waits on a different flag.
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs1,
                                 Thread::REALTIME_MIN_PRI,
                                 Thread::REALTIME,
                                 0,
                                 gThreads[0]));
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs2,
                                 Thread::REALTIME_MAX_PRI,
                                 Thread::REALTIME,
                                 1,
                                 gThreads[1]));

    // Wait for a relatively long time to avoid racing.
    Clock::spinWait(0.25 * Clock::NS_IN_S);

    // At this point no threads have run, so all return times are unset.
    CHECK_EQUAL(0, gArgs1.tReturnNs);
    CHECK_EQUAL(0, gArgs2.tReturnNs);

    // Release lower priority thread from its spin and wait for it to complete.
    // This succeeds because the other thread, though still spinning and higher
    // priority, is on a different core.
    gArgs1.flag = true;
    CHECK_SUCCESS(Thread::await(gThreads[0], nullptr));

    // At this point only the lower priority thread has set its return time.
    CHECK_TRUE(gArgs1.tReturnNs != 0);
    CHECK_EQUAL(0, gArgs2.tReturnNs);

    // Release and wait on higher priority thread.
    gArgs2.flag = true;
    CHECK_SUCCESS(Thread::await(gThreads[1], nullptr));
}
