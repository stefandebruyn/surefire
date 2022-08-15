////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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
/// @file  sf/core/utest/UTestThread.cpp
/// @brief Unit tests for Thread.
////////////////////////////////////////////////////////////////////////////////

#include "UTestThreadCommon.hpp"
#include "sf/pal/Clock.hpp"

/////////////////////////////////// Global /////////////////////////////////////

Thread gTestThreads[gTestMaxThreads];

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Thread that verifies the core it's running on.
///
/// @param[in] kArgs  Expected core number, reinterpreted as void*.
///
/// @retval SUCCESS    Thread is running on expected core.
/// @retval E_THR_AFF  Thread is not running on expected core.
///
static Result checkCore(void* kArgs)
{
    const U64 expectCore = reinterpret_cast<U64>(kArgs);
    return ((expectCore == Thread::currentCore()) ? SUCCESS : E_THR_AFF);
}

///
/// @brief Thread that always returns an error.
///
/// @param[in] kArgs  Unused.
///
/// @retval E_THR_NULL  Always.
///
static Result returnError(void* kArgs)
{
    (void) kArgs;
    return E_THR_NULL;
}

///
/// @brief Thread that spinwaits for a bit and then sets a flag.
///
/// @param[in] kArgs  bool* flag to set, reinterpreted as void*.
///
/// @retval SUCCESS  Always.
///
static Result spinSetFlag(void* kArgs)
{
    Clock::spinWait(0.1 * Clock::NS_IN_S);
    return setFlag(kArgs);
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for Thread.
///
TEST_GROUP(Thread)
{
    void teardown()
    {
        threadTestTeardown();
    }
};

///
/// @test Invoking methods on an uninitialized thread returns an error.
///
TEST(Thread, Uninitialized)
{
    Thread thread;
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

///
/// @test Thread is uninitialized after awaiting it.
///
TEST(Thread, UninitializedAfterAwait)
{
    Thread thread;
    CHECK_SUCCESS(Thread::init(nop,
                               nullptr,
                               Thread::REALTIME_MIN_PRI,
                               Thread::Policy::REALTIME,
                               0,
                               thread));
    CHECK_SUCCESS(thread.await(nullptr));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

///
/// @test Thread can be reused after awaiting it.
///
TEST(Thread, Reuse)
{
    CHECK_SUCCESS(Thread::init(nop,
                               nullptr,
                               Thread::REALTIME_MIN_PRI,
                               Thread::Policy::REALTIME,
                               0,
                               gTestThreads[0]));
    CHECK_SUCCESS(gTestThreads[0].await(nullptr));
    CHECK_SUCCESS(Thread::init(nop,
                               nullptr,
                               Thread::REALTIME_MIN_PRI,
                               Thread::Policy::REALTIME,
                               0,
                               gTestThreads[0]));
}

///
/// @test Thread::await() returns the thread result.
///
TEST(Thread, ReturnResult)
{
    Thread thread;
    CHECK_SUCCESS(Thread::init(returnError,
                               nullptr,
                               Thread::REALTIME_MIN_PRI,
                               Thread::Policy::REALTIME,
                               0,
                               thread));
    Result threadRes = SUCCESS;
    CHECK_SUCCESS(thread.await(&threadRes));
    CHECK_ERROR(E_THR_NULL, threadRes);
}

///
/// @test Threads are successfully created on all cores.
///
TEST(Thread, AffinityRange)
{
    for (U8 i = 0; i < Thread::numCores(); ++i)
    {
        Thread thread;
        CHECK_SUCCESS(Thread::init(checkCore,
                                   reinterpret_cast<void*>(i),
                                   Thread::REALTIME_MIN_PRI,
                                   Thread::Policy::REALTIME,
                                   i,
                                   thread));
        Result threadRes = -1;
        CHECK_SUCCESS(thread.await(&threadRes));
        CHECK_TEXT((threadRes == SUCCESS), "thread was on an unexpected core");
    }
}

///
/// @test Threads with no affinity are successfully created.
///
TEST(Thread, AffinityAllCores)
{
    // Array of flags to be set by threads.
    bool flags[gTestMaxThreads] = {};

    // Create max number of threads.
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        CHECK_SUCCESS(Thread::init(setFlag,
                                   &flags[i],
                                   Thread::REALTIME_MIN_PRI,
                                   Thread::REALTIME,
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

///
/// @test Destructing an initialized thread waits for it to terminate.
///
TEST(Thread, DestructInitialized)
{
    bool flag = false;
    {
        Thread thread;
        CHECK_SUCCESS(Thread::init(spinSetFlag,
                                   &flag,
                                   Thread::REALTIME_MIN_PRI,
                                   Thread::REALTIME,
                                   0,
                                   thread));
    }
    CHECK_TRUE(flag);
}

///
/// @test Destructing an uninitialized thread does nothing.
///
TEST(Thread, DestructUninitialized)
{
    {
        Thread thread;
    }
}

///
/// @test Initializing a thread twice returns an error.
///
TEST(Thread, ErrorReinitialize)
{
    bool flags[2] = {};
    CHECK_SUCCESS(Thread::init(setFlag,
                               &flags[0],
                               Thread::REALTIME_MIN_PRI,
                               Thread::REALTIME,
                               0,
                                 gTestThreads[0]));
    CHECK_ERROR(E_THR_REINIT,
                Thread::init(setFlag,
                             &flags[1],
                             Thread::REALTIME_MIN_PRI,
                             Thread::REALTIME,
                             0,
                             gTestThreads[0]));
    CHECK_SUCCESS(gTestThreads[0].await(nullptr));
    CHECK_TRUE(flags[0]);
    CHECK_TRUE(!flags[1]);
}

///
/// @test Initializing a thread with a null function returns an error.
///
TEST(Thread, ErrorNullFunction)
{
    Thread thread;
    CHECK_ERROR(E_THR_NULL,
                Thread::init(nullptr,
                             nullptr,
                             Thread::REALTIME_MIN_PRI,
                             Thread::REALTIME,
                             0,
                             thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

///
/// @test Initializing a thread with an invalid policy returns an error.
///
TEST(Thread, ErrorInvalidPolicy)
{
    Thread thread;
    CHECK_ERROR(E_THR_POL,
                Thread::init(nop,
                             nullptr,
                             Thread::REALTIME_MIN_PRI,
                             static_cast<Thread::Policy>(0xFF),
                             0,
                             thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

///
/// @test Initializing a thread with an invalid affinity returns an error.
///
TEST(Thread, ErrorInvalidAffinity)
{
    Thread thread;
    CHECK_ERROR(E_THR_AFF,
                Thread::init(nop,
                             nullptr,
                             Thread::REALTIME_MIN_PRI,
                             Thread::REALTIME,
                             Thread::numCores(),
                             thread));
    CHECK_ERROR(E_THR_UNINIT, thread.await(nullptr));
}

///
/// @test Current thread attributes are set successfully.
///
TEST(Thread, SetCurrentThread)
{
    for (U8 i = 0; i < Thread::numCores(); ++i)
    {
        CHECK_SUCCESS(Thread::set(Thread::REALTIME_MIN_PRI,
                                  Thread::REALTIME,
                                  0));
    }

    CHECK_SUCCESS(Thread::set(Thread::REALTIME_MIN_PRI,
                              Thread::REALTIME,
                              Thread::ALL_CORES));
}
