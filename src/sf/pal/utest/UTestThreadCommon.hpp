#ifndef SF_UTEST_THREAD_COMMON_HPP
#define SF_UTEST_THREAD_COMMON_HPP

#include "sf/pal/Thread.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

constexpr U32 gTestMaxThreads = 16;

extern Thread gTestThreads[gTestMaxThreads];

/////////////////////////////////// Helpers ////////////////////////////////////

inline void threadTestTeardown()
{
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        (void) gTestThreads[i].await(nullptr);
    }
}

Result nop(void* kArgs);

Result setFlag(void* kArgs);

#endif
