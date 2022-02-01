#include "ThreadPrivate.hpp"

ThreadSlot gThreadSlots[MAX_THREADS];

void* pthreadWrapper(void* kArgs)
{
    PthreadWrapperArgs* wrapperArgs = (PthreadWrapperArgs*) kArgs;
    const Result res = (*wrapperArgs->func)(wrapperArgs->args);
    return reinterpret_cast<void*>(res);
}

Result Thread::create(const Function kFunc,
                      void* const kArgs,
                      const I32 kPriority,
                      const Policy kPolicy,
                      const U8 kAffinity,
                      I32& kThread)
{
    // Look for an empty slot to store the thread info.
    pthread_t* pthread = nullptr;
    U32 threadSlot = 0;
    for (; threadSlot < MAX_THREADS; ++threadSlot)
    {
        if (gThreadSlots[threadSlot].used == false)
        {
            // Empty slot found. Will be marked used once we successfully create
            // the thread without an error occurring.
            pthread = &gThreadSlots[threadSlot].pthread;
            break;
        }
    }

    if (pthread == nullptr)
    {
        // No empty slot- maximum number of threads reached.
        return E_THR_MAX;
    }

    // Initialize thread attributes.
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0)
    {
        return E_THR_INIT_ATTR;
    }

    // Map policy onto pthread constant.
    I32 schedPolicy = -1;
    switch (kPolicy)
    {
        case DEFAULT_SCHED:
            schedPolicy = SCHED_OTHER;
            break;

        case REALTIME:
            schedPolicy = SCHED_FIFO;
            break;

        default:
            return E_THR_POL;
    }

    // Set thread scheduling policy.
    if (pthread_attr_setschedpolicy(&attr, schedPolicy) != 0)
    {
        return E_THR_POL;
    }

    // `SCHED_OTHER` requires using a static priority of 0, so only set the
    // user-specified thread priority when using non-default scheduling policy.
    if (kPolicy != DEFAULT_SCHED)
    {
        // Set thread priority.
        sched_param param = {};
        param.__sched_priority = kPriority;
        if (pthread_attr_setschedparam(&attr, &param) != 0)
        {
            return E_THR_PRI;
        }

        // Set thread to use the sched param priority instead of inheriting the
        // priority of the parent thread.
        if (pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) != 0)
        {
            return E_THR_INH_PRI;
        }
    }

    // Set thread affinity.
    if (kAffinity != NO_AFFINITY)
    {
        cpu_set_t cpuSet;
        CPU_ZERO(&cpuSet);
        CPU_SET(kAffinity, &cpuSet);
        if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuSet) != 0)
        {
            return E_THR_AFF;
        }
    }

    // Store thread wrapper arguments in thread slot where the thread can access
    // them.
    gThreadSlots[threadSlot].wrapperArgs.func = kFunc;
    gThreadSlots[threadSlot].wrapperArgs.args = kArgs;

    // Start the thread.
    if (pthread_create(pthread,
                       &attr,
                       pthreadWrapper,
                       &gThreadSlots[threadSlot].wrapperArgs) != 0)
    {
        return E_THR_CREATE;
    }

    // If we got this far, thread was successfully created- mark thread slot
    // used and return thread descriptor.
    gThreadSlots[threadSlot].used = true;
    kThread = threadSlot;

    // Destroy thread attributes since we're done with it. This should never
    // fail according to POSIX.
    if (pthread_attr_destroy(&attr) != 0)
    {
        return E_THR_DTRY_ATTR;
    }

    return SUCCESS;
}

Result Thread::await(const I32 kThread, Result* kThreadRes)
{
    // Check that thread descriptor is in range.
    if ((kThread < 0) || (kThread >= MAX_THREADS))
    {
        return E_THR_RANGE;
    }

    // Check that thread slot is in use.
    if (gThreadSlots[kThread].used == false)
    {
        return E_THR_EXIST;
    }

    // Join thread.
    void* threadRet = nullptr;
    if (pthread_join(gThreadSlots[kThread].pthread, &threadRet) != 0)
    {
        return E_THR_AWAIT;
    }

    if (kThreadRes != nullptr)
    {
        // Return thread result to caller.
        static_assert(sizeof(threadRet) >= sizeof(kThreadRes));
        *kThreadRes = *((Result*) &threadRet);
    }

    // Clear the thread slot.
    gThreadSlots[kThread] = {};

    return SUCCESS;
}
