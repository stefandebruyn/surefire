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
/// @file  sf/core/utest/UTestThreadFair.cpp
/// @brief Unit tests for the fair thread scheduling policy.
////////////////////////////////////////////////////////////////////////////////

#include "UTestThreadCommon.hpp"

///
/// @brief Unit tests for the fair thread scheduling policy.
///
/// @remark Gauging whether or not the policy is actually "fair" is hard, so
/// these tests are mostly a smoketest to check that fair threads can be
/// created, run, don't crash, etc.
///
TEST_GROUP(ThreadFair)
{
    void teardown()
    {
        threadTestTeardown();
    }
};

///
/// @test Fair threads are successfully created with all valid priorities.
///
TEST(ThreadFair, PriorityRange)
{
    CHECK_TRUE(Thread::FAIR_MIN_PRI <= Thread::FAIR_MAX_PRI);

    for (I32 i = Thread::FAIR_MIN_PRI; i <= Thread::FAIR_MAX_PRI; ++i)
    {
        bool flag = false;
        CHECK_SUCCESS(Thread::init(setFlag,
                                   &flag,
                                   i,
                                   Thread::FAIR,
                                   Thread::ALL_CORES,
                                   gTestThreads[0]));
        Result threadRes = -1;
        CHECK_SUCCESS(gTestThreads[0].await(&threadRes));
        CHECK_SUCCESS(threadRes);
        CHECK_TRUE(flag);
    }
}

///
/// @test Creating a fair thread with too low of a priority fails.
///
TEST(ThreadFair, PriorityTooLow)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::init(nop,
                             nullptr,
                             (Thread::FAIR_MIN_PRI - 1),
                             Thread::FAIR,
                             Thread::ALL_CORES,
                             gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}

///
/// @test Creating a fair thread with too high of a priority fails.
///
TEST(ThreadFair, PriorityTooHigh)
{
    CHECK_ERROR(E_THR_PRI,
                Thread::init(nop,
                             nullptr,
                             (Thread::FAIR_MAX_PRI + 1),
                             Thread::FAIR,
                             Thread::ALL_CORES,
                             gTestThreads[0]));
    CHECK_ERROR(E_THR_UNINIT, gTestThreads[0].await(nullptr));
}
