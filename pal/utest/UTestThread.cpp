#include "UTestThreadCommon.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

I32 gThreads[gThreadsSize];

/////////////////////////////////// Helpers ////////////////////////////////////

static Result checkCore(void* kArgs)
{
    const U8 expectCore = *((U8*) &kArgs);
    return ((expectCore == Thread::currentCore()) ? SUCCESS : E_THR_AFF);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(Thread)
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

TEST(Thread, CreateMaxThreads)
{
    // Array of flags to be set by threads.
    bool flags[Thread::MAX_THREADS] = {};

    // Create max number of threads.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flags[i],
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     0,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(setFlag,
                               &flags[0],
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               0,
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
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     0,
                                     gThreads[i]));
    }

    // Creating another thread fails.
    CHECK_ERROR(E_THR_MAX,
                Thread::create(setFlag,
                               &flags[0],
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               0,
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

TEST(Thread, AffinityRange)
{
    for (U32 i = 0; i < Thread::numCores(); ++i)
    {
        CHECK_SUCCESS(Thread::create(checkCore,
                                     reinterpret_cast<void*>(i),
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     i,
                                     gThreads[0]));
        Result threadRes = -1;
        CHECK_SUCCESS(Thread::await(gThreads[0], &threadRes));
        CHECK_TEXT((threadRes == SUCCESS), "thread was on an unexpected core");
    }
}

TEST(Thread, AffinityAllCores)
{
    // Array of flags to be set by threads.
    bool flags[Thread::MAX_THREADS] = {};

    // Create max number of threads.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flags[i],
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     Thread::ALL_CORES,
                                     gThreads[i]));
    }

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

TEST(Thread, ErrorNullFunction)
{
    CHECK_ERROR(E_THR_NULL,
                Thread::create(nullptr,
                               nullptr,
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               0,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}

TEST(Thread, ErrorInvalidPolicy)
{
    CHECK_ERROR(E_THR_POL,
                Thread::create(noop,
                               nullptr,
                               Thread::TEST_PRI,
                               static_cast<Thread::Policy>(0xFF),
                               0,
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}

TEST(Thread, ErrorInvalidAffinity)
{
    CHECK_ERROR(E_THR_AFF,
                Thread::create(noop,
                               nullptr,
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               Thread::numCores(),
                               gThreads[0]));
    CHECK_EQUAL(-1, gThreads[0]);
}
