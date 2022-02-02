#include "pal/Thread.hpp"
#include "pal/Clock.hpp"
#include "UTest.hpp"

inline constexpr U32 gThreadsSize = 16;

static I32 gThreads[gThreadsSize];

struct ThreadArgs final
{
    bool flag;
    U64 time;
};

static ThreadArgs gArgs1;
static ThreadArgs gArgs2;
static ThreadArgs gArgs3;

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
    args->time = Clock::nanoTime();
    return SUCCESS;
}

static Result recordTime(void* kArgs)
{
    ThreadArgs* const args = (ThreadArgs*) kArgs;
    args->time = Clock::nanoTime();
    return SUCCESS;
}

TEST_GROUP(ThreadRealTime)
{
    void setup()
    {
        // Check that `gThreads` array is large enough to store the maximum
        // number of thread descriptors. This is necessary since the array
        // cannot be statically sized according to `Thread::MAX_THREADS`, and
        // we want to avoid allocating memory in this test to keep it portable.
        if (Thread::MAX_THREADS > gThreadsSize)
        {
            FAIL("increase `gThreadsSize` to be >= `Thread::MAX_THREADS`");
        }

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

TEST(ThreadRealTime, RealTime)
{
    CHECK_SUCCESS(Thread::create(spinOnFlagAndRecordTime,
                                 &gArgs1,
                                 Thread::REALTIME_MIN_PRI,
                                 Thread::REALTIME,
                                 0,
                                 gThreads[0]));
    CHECK_SUCCESS(Thread::create(recordTime,
                                 &gArgs2,
                                 (Thread::REALTIME_MIN_PRI + 1),
                                 Thread::REALTIME,
                                 0,
                                 gThreads[1]));
    CHECK_SUCCESS(Thread::create(recordTime,
                                 &gArgs3,
                                 (Thread::REALTIME_MIN_PRI + 2),
                                 Thread::REALTIME,
                                 0,
                                 gThreads[2]));

    CHECK_EQUAL(0, gArgs1.time);
    CHECK_EQUAL(0, gArgs2.time);
    CHECK_EQUAL(0, gArgs3.time);

    gArgs1.flag = true;

    CHECK_SUCCESS(Thread::await(gThreads[0], nullptr));
    CHECK_SUCCESS(Thread::await(gThreads[1], nullptr));
    CHECK_SUCCESS(Thread::await(gThreads[2], nullptr));

    CHECK_TRUE(gArgs1.time < gArgs2.time);
    CHECK_TRUE(gArgs2.time < gArgs3.time);
}
