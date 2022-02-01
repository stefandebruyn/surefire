#ifndef SFA_LINUX_THREAD_PRIVATE_HPP
#define SFA_LINUX_THREAD_PRIVATE_HPP

#include <pthread.h>

#include "pal/Thread.hpp"
#include "sfa/BasicTypes.hpp"

inline constexpr U32 MAX_THREADS = 16;

struct PthreadWrapperArgs
{
    Thread::Function func;
    void* args;
};

struct ThreadSlot
{
    pthread_t pthread;
    PthreadWrapperArgs wrapperArgs;
    bool used;
};

extern ThreadSlot gThreadSlots[MAX_THREADS];

void* pthreadWrapper(void* kArgs);

#endif
