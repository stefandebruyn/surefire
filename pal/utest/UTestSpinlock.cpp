#include "pal/Clock.hpp"
#include "pal/Spinlock.hpp"
#include "UTestThreadCommon.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

struct ThreadArgs
{
    Spinlock lock;
    U64 increments;
    U64 counter;
};

static Result atomicIncrement(void* kArgs)
{
    ThreadArgs* const args = static_cast<ThreadArgs*>(kArgs);

    for (U64 i = 0; i < args->increments; ++i)
    {
        // Acquire lock.
        Result lockRes = args->lock.acquire();
        if (lockRes != SUCCESS)
        {
            return lockRes;
        }

        ++args->counter;

        // Release lock.
        lockRes = args->lock.release();
        if (lockRes != SUCCESS)
        {
            return lockRes;
        }
    }

    return SUCCESS;
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(Spinlock)
{
    void teardown()
    {
        threadTestTeardown();
    }
};

TEST(Spinlock, Uninitialized)
{
    Spinlock lock;
    CHECK_ERROR(E_SLK_UNINIT, lock.acquire());
    CHECK_ERROR(E_SLK_UNINIT, lock.release());
}

TEST(Spinlock, MutualExclusion)
{
    // Thread will increment the counter once.
    ThreadArgs args = {};
    args.increments = 1;

    // Create spinlock and acquire it.
    CHECK_SUCCESS(Spinlock::create(args.lock));
    CHECK_SUCCESS(args.lock.acquire());

    // Create thread to increment counter. It will spin on the lock without
    // incrementing the counter since the unit test thread holds the lock.
    CHECK_SUCCESS(Thread::create(atomicIncrement,
                                 &args,
                                 Thread::TEST_PRI,
                                 Thread::TEST_POLICY,
                                 0,
                                 gTestThreads[0]));

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // Counter is still 0.
    CHECK_EQUAL(0, args.counter);

    // Release lock.
    CHECK_SUCCESS(args.lock.release());

    // Wait for thread to finish.
    Result threadRes = -1;
    CHECK_SUCCESS(gTestThreads[0].await(&threadRes));
    CHECK_SUCCESS(threadRes);

    // Counter is now 1.
    CHECK_EQUAL(1, args.counter);
}

/// @note This test is only valid if threads can contend for the counter, so
///       either `Thread::TEST_POLICY` is fair or the platform is multicore.
TEST(Spinlock, AtomicUpdates)
{
    // Make the current thread real-time, max priority, and run on any core.
    // This usually makes the test run significantly faster on systems where
    // `Thread::TEST_POLICY` is fair, since the unit test thread won't be
    // constantly preempted by the fair threads it creates. Ignore any errors
    // in case the current platform doesn't support real-time.
    (void) Thread::set(Thread::REALTIME_MAX_PRI,
                       Thread::REALTIME,
                       Thread::ALL_CORES);

    // Each thread will increment the counter 1000000 times.
    ThreadArgs args = {};
    args.increments = 1000000;

    // Create spinlock and acquire it.
    CHECK_SUCCESS(Spinlock::create(args.lock));
    CHECK_SUCCESS(args.lock.acquire());

    // Create threads. They will spin on the lock without updating the counter
    // since the unit test thread holds the lock. Threads are spread out across
    // cores to maximize contention of the counter.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        CHECK_SUCCESS(Thread::create(atomicIncrement,
                                     &args,
                                     Thread::TEST_PRI,
                                     Thread::TEST_POLICY,
                                     (i % Thread::numCores()),
                                     gTestThreads[i]));
    }

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // At this point no threads have run, so the counter is still 0.
    CHECK_EQUAL(0, args.counter);

    // Release threads from spinwait.
    CHECK_SUCCESS(args.lock.release());

    // Wait for threads to finish.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        CHECK_SUCCESS(gTestThreads[i].await(nullptr));
    }

    // Counter had no lost updates.
    const U64 expectCounter = (gTestMaxThreads * args.increments);
    CHECK_EQUAL(expectCounter, args.counter);
}
