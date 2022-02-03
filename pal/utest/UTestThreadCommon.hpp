#ifndef SFA_UTEST_THREAD_COMMON_HPP
#define SFA_UTEST_THREAD_COMMON_HPP

#include "pal/Thread.hpp"
#include "UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

inline constexpr U32 gTestMaxThreads = 16;

extern Thread gTestThreads[gTestMaxThreads];

/////////////////////////////////// Helpers ////////////////////////////////////

inline void threadTestTeardown()
{
    for (U32 i = 0; i < gTestMaxThreads; ++i)
    {
        gTestThreads[i].await(nullptr);
    }
}

static Result nop(void* kArgs)
{
    return SUCCESS;
}

static Result setFlag(void* kArgs)
{
    *((bool*) kArgs) = true;
    return SUCCESS;
}

#endif
