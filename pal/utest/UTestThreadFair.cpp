#include "pal/Thread.hpp"
#include "UTest.hpp"

inline constexpr U32 gThreadsSize = 16;

static I32 gThreads[gThreadsSize];

static Result noop(void* kArgs)
{
    return SUCCESS;
}

static Result flagSetter(void* kArgs)
{
    *((bool*) kArgs) = true;
    return SUCCESS;
}

TEST_GROUP(ThreadFair)
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

TEST(ThreadFair, CreateMaxThreads)
{
    // Array of flags to be set by threads.
    bool flags[Thread::MAX_THREADS + 1] = {};

    // Create max number of threads.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(flagSetter,
                                     &flags[i],
                                     Thread::FAIR_MIN_PRI,
                                     Thread::FAIR,
                                     Thread::NO_AFFINITY,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(flagSetter,
                               &flags[0],
                               Thread::FAIR_MIN_PRI,
                               Thread::FAIR,
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
        CHECK_SUCCESS(Thread::create(flagSetter,
                                     &flags[i],
                                     Thread::FAIR_MIN_PRI,
                                     Thread::FAIR,
                                     Thread::NO_AFFINITY,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(flagSetter,
                               &flags[0],
                               Thread::FAIR_MIN_PRI,
                               Thread::FAIR,
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

TEST(ThreadFair, PriorityRange)
{
    for (I32 i = Thread::FAIR_MIN_PRI; i <= Thread::FAIR_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::create(flagSetter,
                                     &flag,
                                     i,
                                     Thread::FAIR,
                                     Thread::NO_AFFINITY,
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
                               Thread::NO_AFFINITY,
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
                               Thread::NO_AFFINITY,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}
