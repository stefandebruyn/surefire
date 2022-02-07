#ifndef SFA_THREAD_HPP
#define SFA_THREAD_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"

#ifdef SFA_PLATFORM_LINUX
#    include <pthread.h>
#endif

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

    /// A valid priority for the current platform that can be used to create
    /// threads in priority-agnostic, low-stakes test code. This should not be
    /// accessed in production code.
    static const I32 TEST_PRI;

    /// A valid scheduling policy for the current platform that can be used to
    /// create threads in policy-agnostic, low-stakes test code. This should not
    /// be accessed in production code.
    static const Policy TEST_POLICY;

    static constexpr U8 ALL_CORES = 0xFF;

    typedef Result (*Function)(void* kArgs);

    /// Starts a thread.
    ///
    /// @note Linux: This function is not thread-safe.
    /// @note Linux: The thread has the default stack bounds chosen by pthread.
    ///
    /// @param[i] kFunc      Thread function. The thread exits when this
    ///                      function returns.
    /// @param[i] kArgs      Thread arguments. This pointer is passed to the
    ///                      thread function as-is. If the pointer is non-null,
    ///                      The data pointed to should be in scope for the
    ///                      lifetime of the thread.
    /// @param[i] kPriority  Thread priority. Valid priority ranges may vary
    ///                      with platform.
    ///                      Linux: This parameter is ignored if a `FAIR` policy
    ///                      is used. The thread is given pthread policy
    ///                      `SCHED_OTHER` with a static priority of 0.
    /// @param[i] kPolicy    Thread scheduling policy.
    /// @param[i] kAffinity  Thread affinity. This should be a zero-indexed CPU
    ///                      core ID.
    /// @param[o] kThread    On success, contains a descriptor used to refer to
    ///                      the created thread.
    ///
    /// @retval SUCCESS          Thread started successfully. Thread descriptor
    ///                          was stored in `kThread`.
    /// @retval E_THR_NULL       Function pointer was null.
    /// @retval E_THR_MAX        Maximum number of threads reached.
    /// @retval E_THR_POL        Failed to set scheduling policy. The policy may
    ///                          be invalid or unsupported by the platform.
    /// @retval E_THR_PRI        Failed to set priority. The priority may be
    ///                          invalid or unsupported by the platform.
    /// @retval E_THR_CREATE     Failed to create thread. This usually means the
    ///                          platform-specific thread creation API call
    ///                          failed.
    ///                          Linux: If the `REALTIME` policy is used, this
    ///                          error may indicate that the process does not
    ///                          have sufficient permissions to create real-time
    ///                          threads.
    /// @retval E_THR_AFF        Failed to set thread affinity. The affinity may
    ///                          be invalid or unsupported by the platform.
    /// @retval E_THR_INIT_ATTR  Linux: Failed to initialize thread attributes.
    /// @retval E_THR_INH_PRI    Linux: Failed to disable thread priority
    ///                          inheritance.
    /// @retval E_THR_DTRY_ATTR  Linux: Failed to destroy thread attributes, but
    ///                          the thread was still created successfully.
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

    Result await(Result* const kThreadRes);

    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;

private:

    bool mInit;

#ifdef SFA_PLATFORM_LINUX
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
