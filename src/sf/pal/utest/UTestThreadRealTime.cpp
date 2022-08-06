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
/// @file  sf/core/utest/UTestThreadRealTime.cpp
/// @brief Unit tests for the real-time thread scheduling policy.
////////////////////////////////////////////////////////////////////////////////

#include "UTestThreadCommon.hpp"
#include "sf/pal/Clock.hpp"

#include <iostream>

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief Test thread arguments.
///
struct ThreadArgs final
{
    bool flag;     ///< Flag to spin on.
    U64 tReturnNs; ///< Set by thread to current time just before returning.
    U64 waitNs;    ///< Time which thread should spinwait before returning.
};

static ThreadArgs gArgs1;
static ThreadArgs gArgs2;
static ThreadArgs gArgs3;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Thread that spins for a time and then records the time of its return.
///
/// @param[in] kArgs  ThreadArgs*, reinterpreted as void*.
///
/// @retval SUCCESS  Always succeeds.
///
static Result spinAndRecordTime(void* kArgs)
{
    ThreadArgs* const args = static_cast<ThreadArgs*>(kArgs);
    Clock::spinWait(args->waitNs);
    args->tReturnNs = Clock::nanoTime();
    return SUCCESS;
}

///
/// @brief Thread that spins on a flag and records the time of its return.
///
/// @param[in] kArgs  ThreadArgs*, reinterpreted as void*.
///
/// @retval SUCCESS  Always succeeds.
///
static Result spinOnFlagAndRecordTime(void* kArgs)
{
    ThreadArgs* const args = static_cast<ThreadArgs*>(kArgs);
    volatile const bool* const flag = &args->flag;
    while (!(*flag));
    args->tReturnNs = Clock::nanoTime();
    return SUCCESS;
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for the real-time thread scheduling policy.
///
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

///
/// @test Real-time threads are successfully created with all valid priorities.
///
TEST(ThreadRealTime, PriorityRange)
{
    CHECK_TRUE(Thread::REALTIME_MIN_PRI <= Thread::REALTIME_MAX_PRI);

    for (I32 i = Thread::REALTIME_MIN_PRI; i <= Thread::REALTIME_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::init(setFlag,
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

///
/// @test Creating a real-time thread with too low of a priority fails.
///
TEST(ThreadRealTime, PriorityTooLow)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::init(nop,
                             nullptr,
                             (Thread::REALTIME_MIN_PRI - 1),
                             Thread::REALTIME,
                             Thread::ALL_CORES,
                             gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}

///
/// @test Creating a real-time thread with too high of a priority fails.
///
TEST(ThreadRealTime, PriorityTooHigh)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::init(nop,
                             nullptr,
                             (Thread::REALTIME_MAX_PRI + 1),
                             Thread::REALTIME,
                             Thread::ALL_CORES,
                             gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}

///
/// @test Real-time threads with the same affinity execute in order of
/// decreasing priority.
///
/// @note This test assumes that a larger priority value corresponds to higher
/// priority.
///
TEST(ThreadRealTime, RealTimeSameAffinity)
{
    // Make the current thread real-time, max priority, and run on core 0 for
    // determinism.
    CHECK_SUCCESS(Thread::set(Thread::REALTIME_MAX_PRI, Thread::REALTIME, 0));

    // Threads 2 and 3 will spin for 100 ms before returning.
    gArgs2.waitNs = (0.1 * Clock::NS_IN_S);
    gArgs3.waitNs = gArgs2.waitNs;

    // Set flag so that thread 1 does not spinwait. It will still not run
    // immediately since it has lower priority than the current thread.
    gArgs1.flag = true;

    // Create 3 real-time threads with descending priorities on the same core.
    // All 3 threads are blocked until the current thread yields. Threads will
    // record the time of their return in the argument structs passed to them.
    CHECK_SUCCESS(Thread::init(spinOnFlagAndRecordTime,
                               &gArgs1,
                               (Thread::REALTIME_MIN_PRI + 2),
                               Thread::REALTIME,
                               0,
                               gTestThreads[0]));
    CHECK_SUCCESS(Thread::init(spinAndRecordTime,
                               &gArgs2,
                               (Thread::REALTIME_MIN_PRI + 1),
                               Thread::REALTIME,
                               0,
                               gTestThreads[1]));
    CHECK_SUCCESS(Thread::init(spinAndRecordTime,
                               &gArgs3,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               0,
                               gTestThreads[2]));

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // At this point no threads have returned, so all return times are unset.
    CHECK_EQUAL(0, gArgs1.tReturnNs);
    CHECK_EQUAL(0, gArgs2.tReturnNs);
    CHECK_EQUAL(0, gArgs3.tReturnNs);

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

///
/// @test Real-time threads with different affinities execute in parallel.
///
/// @note This test requires that affinities 0 and 1 be valid on the current
/// platform.
///
TEST(ThreadRealTime, RealTimeDifferentAffinity)
{
    // Make the current thread real-time, max priority, and run on core 0 for
    // determinism.
    CHECK_SUCCESS(Thread::set(Thread::REALTIME_MAX_PRI, Thread::REALTIME, 0));

    // Create 2 real-time threads with different priorities on different cores.
    CHECK_SUCCESS(Thread::init(spinOnFlagAndRecordTime,
                               &gArgs1,
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               0,
                               gTestThreads[0]));
    CHECK_SUCCESS(Thread::init(spinOnFlagAndRecordTime,
                               &gArgs2,
                               (Thread::REALTIME_MIN_PRI + 1),
                               Thread::REALTIME,
                               1,
                               gTestThreads[1]));

    // Wait a relatively long time to avoid racing thread creation.
    Clock::spinWait(0.1 * Clock::NS_IN_S);

    // At this point no threads have returned, so all return times are unset.
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
    CHECK_TRUE(gArgs2.tReturnNs != 0);
}
