////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/pal/utest/UTestSpinlock.cpp
/// @brief Unit tests for Spinlock.
////////////////////////////////////////////////////////////////////////////////

#include "UTestThreadCommon.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Spinlock.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Test thread arguments.
///
struct ThreadArgs
{
    Spinlock lock;  ///< Spinlock protecting counter access.
    U64 increments; ///< Number of times to increment counter.
    U64 counter;    ///< Counter.
};

///
/// @brief Thread that atomically increments a counter some number of times.
///
/// @param[in] kArgs  Thread arguments.
///
/// @retval SUCCESS  Successfully incremented.
/// @retval [other]  Error from acquiring or releasing spinlock.
///
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

///
/// @brief Unit tests for Spinlock.
///
TEST_GROUP(Spinlock)
{
    void teardown()
    {
        threadTestTeardown();
    }
};

///
/// @test Acquiring or releasing an uninitialized spinlock returns an error.
///
TEST(Spinlock, Uninitialized)
{
    Spinlock lock;
    CHECK_ERROR(E_SLK_UNINIT, lock.acquire());
    CHECK_ERROR(E_SLK_UNINIT, lock.release());
}

///
/// @test Initializing a spinlock twice returns an error.
///
TEST(Spinlock, ErrorReinitialize)
{
    Spinlock lock;
    CHECK_SUCCESS(Spinlock::init(lock));
    CHECK_ERROR(E_SLK_REINIT, Spinlock::init(lock));
}

///
/// @test Spinlock provides mutual exclusion.
///
TEST(Spinlock, MutualExclusion)
{
    // Thread will increment the counter once.
    ThreadArgs args{};
    args.increments = 1;

    // Create spinlock and acquire it.
    CHECK_SUCCESS(Spinlock::init(args.lock));
    CHECK_SUCCESS(args.lock.acquire());

    // Create thread to increment counter. It will spin on the lock without
    // incrementing the counter since the unit test thread holds the lock.
    CHECK_SUCCESS(Thread::init(atomicIncrement,
                               &args,
                               Thread::REALTIME_MIN_PRI,
                               Thread::Policy::REALTIME,
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

///
/// @test Updates made atomic via a spinlock around a contended critical
/// section.
///
/// @note This test is only valid if the platform is multicore, so that
/// real-time threads can contend for the counter.
///
TEST(Spinlock, AtomicUpdates)
{
    // Each thread will increment the counter 1000000 times.
    ThreadArgs args{};
    args.increments = 1000000;

    // Create spinlock and acquire it.
    CHECK_SUCCESS(Spinlock::init(args.lock));
    CHECK_SUCCESS(args.lock.acquire());

    // Create threads. They will spin on the lock without updating the counter
    // since the unit test thread holds the lock. Threads are spread out across
    // cores to maximize contention of the counter.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        CHECK_SUCCESS(Thread::init(atomicIncrement,
                                   &args,
                                   Thread::REALTIME_MIN_PRI,
                                   Thread::Policy::REALTIME,
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
