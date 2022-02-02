#ifndef SFA_UTEST_THREAD_COMMON_HPP
#define SFA_UTEST_THREAD_COMMON_HPP

#include "pal/Thread.hpp"
#include "UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

inline constexpr U32 gThreadsSize = 16;

extern I32 gThreads[gThreadsSize];

/////////////////////////////////// Helpers ////////////////////////////////////

inline void threadTestSetup()
{
    // Check that `gThreads` array is large enough to store the maximum
    // number of thread descriptors. This is necessary since the array
    // cannot be statically sized according to `Thread::MAX_THREADS`, and
    // we want to avoid allocating memory in this test to keep it portable.
    // If this check fails, increase `gThreadsSize`.
    CHECK_TEXT(gThreadsSize >= Thread::MAX_THREADS,
               "increase `gThreadsSize` to be >= `Thread::MAX_THREADS");

    // Reset global thread descriptors.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        gThreads[i] = -1;
    }
}

inline void threadTestTeardown()
{
    // Attempt to wait on all threads in case the test failed with threads
    // still alive. If the test passed, these waits fail silently.
    for (U32 i = 0; i < Thread::MAX_THREADS; ++i)
    {
        Thread::await(gThreads[i], nullptr);
    }
}

static Result noop(void* kArgs)
{
    return SUCCESS;
}

static Result setFlag(void* kArgs)
{
    *((bool*) kArgs) = true;
    return SUCCESS;
}

#endif
