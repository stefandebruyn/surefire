#ifndef SF_THREAD_HPP
#define SF_THREAD_HPP

#ifdef SF_PLATFORM_LINUX
#    include <pthread.h>
#endif

#include "sf/core/Result.hpp"
#include "sf/core/BasicTypes.hpp"

class Thread final
{
public:

    enum Policy : U8
    {
        FAIR,
        REALTIME
    };

    static const I32 FAIR_MIN_PRI;

    static const I32 FAIR_MAX_PRI;

    static const I32 REALTIME_MIN_PRI;

    static const I32 REALTIME_MAX_PRI;

    static constexpr U8 ALL_CORES = 0xFF;

    typedef Result (*Function)(void* kArgs);

    static Result create(const Function kFunc,
                         void* const kArgs,
                         const I32 kPriority,
                         const Policy kPolicy,
                         const U8 kAffinity,
                         Thread& kThread);

    static U8 numCores();

    static U8 currentCore();

    static Result set(const I32 kPriority,
                      const Policy kPolicy,
                      const U8 kAffinity);

    Thread();

    ~Thread();

    Result await(Result* const kThreadRes);

    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;

private:

    bool mInit;

#ifdef SF_PLATFORM_LINUX
    pthread_t mPthread;

    struct PthreadWrapperArgs final
    {
        Function func;
        void* args;
    };

    PthreadWrapperArgs mWrapperArgs;

    static void* pthreadWrapper(void* kArgs);

    static Result getPthreadPolicy(const Policy kPolicy,
                                   const I32 kPriority,
                                   I32& kPthreadPolicy);
#endif
};

#endif