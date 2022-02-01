#ifndef SFA_THREAD_HPP
#define SFA_THREAD_HPP

#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

namespace Thread
{
    enum Policy : U8
    {
        DEFAULT_SCHED,
        REALTIME
    };

    inline constexpr U8 NO_AFFINITY = 0xFF;

    typedef Result (*Function)(void* kArgs);

    /// Starts a thread.
    ///
    /// @note Linux: The thread has the default stack bounds as chosen by
    ///       pthread.
    /// @note Linux: A maximum of 16 threads may be alive at once.
    ///
    /// @param[i] kFunc      Thread function. The thread exits when this
    ///                      function returns.
    /// @param[i] kArgs      Thread arguments. This pointer is passed to the
    ///                      thread function as-is. The data pointed to should
    ///                      be in scope for the lifetime of the thread.
    /// @param[i] kPriority  Thread priority. Valid priority ranges may vary
    ///                      with platform.
    ///                      Linux: This is ignored if a `DEFAULT_SCHED` policy
    ///                      is used; the thread is given pthread policy
    ///                      `SCHED_OTHER` with a static priority of 0.
    /// @param[i] kPolicy    Thread scheduling policy.
    /// @param[i] kAffinity  Thread affinity. This should be a zero-indexed CPU
    ///                      core ID.
    /// @param[o] kThread    On success, contains a descriptor used to refer to
    ///                      the created thread.
    ///
    /// @retval SUCCESS          Thread started successfully. Thread descriptor
    ///                          was stored in `kThread`.
    /// @retval E_THR_MAX        Maximum number of threads reached.
    /// @retval E_THR_POL        Failed to set scheduling policy. The policy may
    ///                          be invalid or unsupported by the platform.
    /// @retval E_THR_PRI        Failed to set priority. The priority may be
    ///                          invalid or unsupported by the platform.
    /// @retval E_THR_CREATE     Failed to create thread. This usually means the
    ///                          platform-specific thread creation API call
    ///                          failed.
    /// @retval E_THR_AFF        Failed to set thread affinity. The affinity may
    ///                          be invalid or unsupported by the platform.
    /// @retval E_THR_INIT_ATTR  Linux: Failed to initialize thread attributes.
    /// @retval E_THR_INH_PRI    Linux: Failed to disable thread priority
    ///                          inheritance.
    /// @retval E_THR_DTRY_ATTR  Linux: Failed to destroy thread attributes, but
    ///                          the thread was still created successfully.
    Result create(const Function kFunc,
                  void* const kArgs,
                  const I32 kPriority,
                  const Policy kPolicy,
                  const U8 kAffinity,
                  I32& kThread);

    Result await(const I32 kThread, Result* kThreadRes);
}

#endif
