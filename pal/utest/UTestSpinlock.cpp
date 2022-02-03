#include "pal/Spinlock.hpp"
#include "UTestThreadCommon.hpp"

struct ThreadArgs
{
    bool start;
    Spinlock lock;
    U64 increments;
    U64 counter;
};

static Result atomicIncrement(void* kArgs)
{
    ThreadArgs* const args = (ThreadArgs* const) kArgs;
    // Spin-wait on start flag.
    while (args->start == false);

    Result res = SUCCESS;
    for (U64 i = 0; i < args->increments; ++i)
    {
        // Acquire lock.
        Result lockRes = args->lock.acquire();
        // Save acquire error if not already storing an error.
        if ((lockRes != SUCCESS) && (res == SUCCESS))
        {
            res = lockRes;
        }

        ++args->counter;

        // Release lock.
        lockRes = args->lock.release();
        // Save release error if not already storing an error.
        if ((lockRes != SUCCESS) && (res == SUCCESS))
        {
            res = lockRes;
        }
    }

    return res;
}

TEST_GROUP(Spinlock)
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

TEST(Spinlock, Atomicity)
{
    ThreadArgs args = {};
    CHECK_SUCCESS(Spinlock::create(args.lock));
    args.increments = 1000000;

    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::create(atomicIncrement,
                                     &args,
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     (i % Thread::numCores()),
                                     gThreads[i]));
    }

    CHECK_EQUAL(0, args.counter);

    args.start = true;

    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        CHECK_SUCCESS(Thread::await(gThreads[i], nullptr));
    }

    const U64 expectCounter = (Thread::MAX_THREADS * args.increments);
    CHECK_EQUAL(expectCounter, args.counter);
}
