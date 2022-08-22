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
/// @file  sf/pal/utest/UTestClock.cpp
/// @brief Unit tests for Clock.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/Clock.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Recursively computes a Fibonacci number. This is used as meaningless
/// work to burn CPU cycles.
///
/// @param[in] kN  Fibonacci number to compute.
///
/// @return kNth Fibonacci number.
///
static U64 fib(const U64 kN)
{
    if (kN <= 1)
    {
        return kN;
    }

    return (fib(kN - 1) + fib(kN - 2));
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for Clock.
///
TEST_GROUP(Clock)
{
};

///
/// @test Clock::nanoTime() is monotonically increasing over a short duration.
///
TEST(Clock, MonotonicNanoTime)
{
    U64 lastTimeNs = Clock::nanoTime();
    for (U32 i = 0; i < 100; ++i)
    {
        // Do some meaningless work before taking another timestamp. Without
        // this, the test occasionally fails on some systems due to the same
        // timestamp being seen twice in a row. This is believed to be caused by
        // some combation of
        //   1. Fast CPUs or very little contention in the system
        //   2. Low system clock resolution
        //   3. Compiler optimizations
        (void) fib(10);
        const U64 curTimeNs = Clock::nanoTime();
        CHECK_TRUE(curTimeNs > lastTimeNs);
        lastTimeNs = curTimeNs;
    }
}
