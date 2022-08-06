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
/// @file  sf/psl/linux/utest/UTestLinuxThreadPriorityRange.cpp
/// @brief Unit tests for thread priority ranges on Linux platforms.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/Thread.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Unit tests for thread priority ranges on Linux platforms.
///
TEST_GROUP(LinuxThreadPriorityRange)
{
};

///
/// @test Fair priority range is correct.
///
TEST(LinuxThreadPriorityRange, FairRange)
{
    CHECK_EQUAL(0, Thread::FAIR_MIN_PRI);
    CHECK_EQUAL(0, Thread::FAIR_MAX_PRI);
}

///
/// @test Real-time priority range is correct.
///
TEST(LinuxThreadPriorityRange, RealTimeRange)
{
    CHECK_EQUAL(1, Thread::REALTIME_MIN_PRI);
    CHECK_EQUAL(99, Thread::REALTIME_MAX_PRI);
}
