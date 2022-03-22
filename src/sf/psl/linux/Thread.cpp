#include <cstring>
#include <sched.h>
#include <sys/sysinfo.h>

#include "sf/pal/Thread.hpp"

const I32 Thread::FAIR_MIN_PRI = 0;

const I32 Thread::FAIR_MAX_PRI = 0;

const I32 Thread::REALTIME_MIN_PRI =
#ifdef SF_PLATFORM_NILRT
    // Priority just above the RCU kernel thread, which has priority 1 on NILRT.
    2;
#else
    // Minimum real-time priority on Linux.
    1;
#endif

const I32 Thread::REALTIME_MAX_PRI =
#ifdef SF_PLATFORM_NILRT
    // Priority just below the software and hardware IRQ kernel threads, which
    // have priorities 14 and 15, respectively, on NILRT.
    13;
#else
    // Maximum real-time priority on Linux.
    99;
#endif

Result Thread::create(const Function kFunc,
                      void* const kArgs,
                      const I32 kPriority,
                      const Policy kPolicy,
                      const U8 kAffinity,
                      Thread& kThread)
{
    // Verify thread is not already initialized.
    if (kThread.mInit)
    {
        return E_THR_REINIT;
    }

    // Check that function is non-null.
    if (kFunc == nullptr)
    {
        return E_THR_NULL;
    }

    // Check that affinity is valid.
    if ((kAffinity != ALL_CORES) && (kAffinity >= numCores()))
    {
        return E_THR_AFF;
    }

    // Initialize thread attributes.
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0)
    {
        return E_THR_INIT_ATTR;
    }

    // Map scheduling policy onto pthread constant.
    I32 schedPolicy = -1;
    const Result res = getPthreadPolicy(kPolicy, kPriority, schedPolicy);
    if (res != SUCCESS)
    {
        return res;
    }

    // Set thread scheduling policy.
    if (pthread_attr_setschedpolicy(&attr, schedPolicy) != 0)
    {
        return E_THR_POL;
    }

    // `SCHED_OTHER` requires using a static priority of 0, so only set the
    // user-specified thread priority when using non-default scheduling policy.
    if (kPolicy != FAIR)
    {
        // Set thread priority.
        sched_param param;
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
    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    if (kAffinity == ALL_CORES)
    {
        for (U8 i = 0; i < numCores(); ++i)
        {
            CPU_SET(i, &cpuSet);
        }
    }
    else
    {
        CPU_SET(kAffinity, &cpuSet);
    }
    if (pthread_attr_setaffinity_np(&attr, sizeof(cpuSet), &cpuSet) != 0)
    {
        return E_THR_AFF;
    }

    // Store thread wrapper arguments in thread object where the thread can
    // access them.
    kThread.mWrapperArgs.func = kFunc;
    kThread.mWrapperArgs.args = kArgs;

    // Start the thread.
    if (pthread_create(&kThread.mPthread,
                       &attr,
                       pthreadWrapper,
                       &kThread.mWrapperArgs) != 0)
    {
        return E_THR_CREATE;
    }

    // Thread successfully created- initialize thread handle.
    kThread.mInit = true;

    // Destroy thread attributes since we're done with them. This should never
    // fail according to POSIX.
    if (pthread_attr_destroy(&attr) != 0)
    {
        return E_THR_DTRY_ATTR;
    }

    return SUCCESS;
}

U8 Thread::numCores()
{
    return get_nprocs();
}

U8 Thread::currentCore()
{
    return static_cast<U8>(sched_getcpu());
}

Result Thread::set(const I32 kPriority,
                   const Policy kPolicy,
                   const U8 kAffinity)
{
    pthread_t me = pthread_self();

    // Map scheduling policy onto pthread constant.
    I32 schedPolicy = -1;
    const Result res = getPthreadPolicy(kPolicy, kPriority, schedPolicy);
    if (res != SUCCESS)
    {
        return res;
    }

    sched_param param;
    param.__sched_priority = kPriority;
    if (pthread_setschedparam(me, schedPolicy, &param) != 0)
    {
        return E_THR_PRI;
    }

    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    if (kAffinity == ALL_CORES)
    {
        for (U8 i = 0; i < numCores(); ++i)
        {
            CPU_SET(i, &cpuSet);
        }
    }
    else
    {
        CPU_SET(kAffinity, &cpuSet);
    }
    if (pthread_setaffinity_np(me, sizeof(cpuSet), &cpuSet) != 0)
    {
        return E_THR_AFF;
    }

    return SUCCESS;
}

Thread::Thread() : mInit(false), mWrapperArgs({nullptr, nullptr})
{
}

Thread::~Thread()
{
    (void) this->await(nullptr);
}

Result Thread::await(Result* const kThreadRes)
{
    if (!mInit)
    {
        return E_THR_UNINIT;
    }

    // Join thread.
    void* threadRes = nullptr;
    if (pthread_join(mPthread, &threadRes) != 0)
    {
        return E_THR_AWAIT;
    }

    if (kThreadRes != nullptr)
    {
        // Return thread result to caller.
        std::memcpy(kThreadRes, &threadRes, sizeof(*kThreadRes));
    }

    // Clear the thread slot.
    mInit = false;

    return SUCCESS;
}

void* Thread::pthreadWrapper(void* kArgs)
{
    PthreadWrapperArgs* const wrapperArgs =
        static_cast<PthreadWrapperArgs*>(kArgs);
    const Result res = (*wrapperArgs->func)(wrapperArgs->args);
    return reinterpret_cast<void*>(res);
}

Result Thread::getPthreadPolicy(const Policy kPolicy,
                                const I32 kPriority,
                                I32& kPthreadPolicy)
{
    switch (kPolicy)
    {
        case FAIR:
            // Check that priority is in range for this policy.
            if ((kPriority < FAIR_MIN_PRI) || (kPriority > FAIR_MAX_PRI))
            {
                return E_THR_PRI;
            }
            kPthreadPolicy = SCHED_OTHER;
            break;

        case REALTIME:
            // Check that priority is in range for this policy.
            if ((kPriority < REALTIME_MIN_PRI)
                || (kPriority > REALTIME_MAX_PRI))
            {
                return E_THR_PRI;
            }
            kPthreadPolicy = SCHED_FIFO;
            break;

        default:
            return E_THR_POL;
    }

    return SUCCESS;
}
