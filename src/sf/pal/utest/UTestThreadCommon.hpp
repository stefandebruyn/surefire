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
/// @file  sf/pal/utest/UTestThreadCommon.cpp
/// @brief Code shared by thread-related unit tests.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_UTEST_THREAD_COMMON_HPP
#define SF_UTEST_THREAD_COMMON_HPP

#include "sf/pal/Thread.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief Maximum number of threads to create at once.
///
constexpr U32 gTestMaxThreads = 16;

///
/// @brief Array of active threads. Any test that creates threads should use
/// the objects in this array.
///
extern Thread gTestThreads[gTestMaxThreads];

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Waits for all active threads to terminate. To be called in the
/// teardown() method of a test that creates threads, so that threads do not
/// linger after a thread test fails and ends early.
///
inline void threadTestTeardown()
{
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        (void) gTestThreads[i].await(nullptr);
    }
}

///
/// @brief Thread that does nothing.
///
/// @param[in] kArgs  Unused.
///
/// @retval SUCCESS  Always.
///
Result nop(void* kArgs);

///
/// @brief Thread that sets a flag.
///
/// @param[in] kArgs  bool* to flag, reinterpreted as void*.
///
/// @retval SUCCESS  Always.
///
Result setFlag(void* kArgs);

#endif
