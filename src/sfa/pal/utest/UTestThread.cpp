#include "UTestThreadCommon.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

Thread gTestThreads[gTestMaxThreads];

/////////////////////////////////// Helpers ////////////////////////////////////

static Result checkCore(void* kArgs)
{
    const U64 expectCore = reinterpret_cast<U64>(kArgs);
    return ((expectCore == Thread::currentCore()) ? SUCCESS : E_THR_AFF);
}

static Result returnError(void* kArgs)
{
    (void) kArgs;
    return E_NULLPTR;
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(Thread)
{
    void teardown()
    {
        threadTestTeardown();
    }
};

TEST(Thread, Uninitialized)
{
    Thread thread;
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

TEST(Thread, UninitializedAfterAwait)
{
    Thread thread;
    CHECK_SUCCESS(Thread::create(nop,
                                 nullptr,
                                 Thread::TEST_PRI,
                                 Thread::TEST_POLICY,
                                 0,
                                 thread));
    CHECK_SUCCESS(thread.await(nullptr));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

TEST(Thread, ReturnResult)
{
    Thread thread;
    CHECK_SUCCESS(Thread::create(returnError,
                                 nullptr,
                                 Thread::TEST_PRI,
                                 Thread::TEST_POLICY,
                                 0,
                                 thread));
    Result threadRes = SUCCESS;
    CHECK_SUCCESS(thread.await(&threadRes));
    CHECK_EQUAL(E_NULLPTR, threadRes);
}

TEST(Thread, AffinityRange)
{
    for (U8 i = 0; i < Thread::numCores(); ++i)
    {
        Thread thread;
        CHECK_SUCCESS(Thread::create(checkCore,
                                     reinterpret_cast<void*>(i),
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     i,
                                     thread));
        Result threadRes = -1;
        CHECK_SUCCESS(thread.await(&threadRes));
        CHECK_TEXT((threadRes == SUCCESS), "thread was on an unexpected core");
    }
}

TEST(Thread, AffinityAllCores)
{
    // Array of flags to be set by threads.
    bool flags[gTestMaxThreads] = {};

    // Create max number of threads.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        CHECK_SUCCESS(Thread::create(setFlag,
                                     &flags[i],
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     Thread::ALL_CORES,
                                     gTestThreads[i]));
    }

    // Wait for threads to finish.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        Result threadRes = -1;
        CHECK_SUCCESS(gTestThreads[i].await(&threadRes));
        CHECK_SUCCESS(threadRes);
        // Flag was set by thread.
        CHECK_TRUE(flags[i]);
    }
}

TEST(Thread, ErrorNullFunction)
{
    Thread thread;
    CHECK_ERROR(E_THR_NULL,
                Thread::create(nullptr,
                               nullptr,
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               0,
                               thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

TEST(Thread, ErrorInvalidPolicy)
{
    Thread thread;
    CHECK_ERROR(E_THR_POL,
                Thread::create(nop,
                               nullptr,
                               Thread::TEST_PRI,
                               static_cast<Thread::Policy>(0xFF),
                               0,
                               thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

TEST(Thread, ErrorInvalidAffinity)
{
    Thread thread;
    CHECK_ERROR(E_THR_AFF,
                Thread::create(nop,
                               nullptr,
                               Thread::TEST_PRI,
                               Thread::TEST_POLICY,
                               Thread::numCores(),
                               thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

TEST(Thread, SetCurrentThread)
{
    for (U8 i = 0; i < Thread::numCores(); ++i)
    {
        CHECK_SUCCESS(Thread::set(Thread::TEST_PRI, Thread::TEST_POLICY, 0));
    }

    CHECK_SUCCESS(Thread::set(Thread::TEST_PRI,
                              Thread::TEST_POLICY,
                              Thread::ALL_CORES));
}