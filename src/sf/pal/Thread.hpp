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
/// @file  sf/pal/Thread.hpp
/// @brief Platform-agnostic threading interface.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_THREAD_HPP
#define SF_THREAD_HPP

#ifdef SF_PLATFORM_LINUX
#    include <pthread.h>
#endif

#include "sf/core/Result.hpp"
#include "sf/core/BasicTypes.hpp"

///
/// @brief Platform-agnostic threading interface.
///
class Thread final
{
public:

    ///
    /// @brief Possible thread scheduling policies.
    ///
    /// @remark Each policy should have its own *_MIN_PRI and *_MAX_PRI
    /// constants in the Thread namespace.
    ///
    enum Policy : U8
    {
        FAIR = 0,     ///< Fair scheduling. "Fair" is implementation-defined.
        REALTIME = 1  ///< Soft or hard real-time scheduling.
    };

    ///
    /// @brief Minimum fair thread priority.
    ///
    static const I32 FAIR_MIN_PRI;

    ///
    /// @brief Maximum fair thread priority.
    ///
    static const I32 FAIR_MAX_PRI;

    ///
    /// @brief Minimum real-time thread priority.
    ///
    static const I32 REALTIME_MIN_PRI;

    ///
    /// @brief Maximum real-time thread priority.
    ///
    static const I32 REALTIME_MAX_PRI;

    ///
    /// @brief Value reserved for no/all-cores affinity.
    ///
    static constexpr U8 ALL_CORES = 0xFF;

    ///
    /// @brief Signature for a thread function.
    ///
    /// @param[in] kArgs  Thread arguments pointer.
    ///
    /// @returns Return code made available to code which waits on the thread.
    ///
    typedef Result (*Function)(void* kArgs);

    static Result init(const Function kFunc,
                       void* const kArgs,
                       const I32 kPriority,
                       const Policy kPolicy,
                       const U8 kAffinity,
                       Thread& kThread);

    ///
    /// @brief Gets the number of cores on the system.
    ///
    /// @returns Number of cores.
    ///
    static U8 numCores();

    ///
    /// @brief Gets the core that the calling thread is currently unning on.
    ///
    /// @returns Core of calling thread.
    ///
    static U8 currentCore();

    ///
    /// @brief Sets attributes of the calling thread.
    ///
    /// @param[in] kPriority  Thread priority.
    /// @param[in] kPolicy    Thread scheduling policy.
    /// @param[in] kAffinity  Thread affinity.
    ///
    /// @retval SUCCESS    Successfully set thread attributes.
    /// @retval E_THR_PRI  kPriority is invalid for the specified policy.
    /// @retval E_THR_POL  kPolicy is invalid.
    /// @retval E_THR_AFF  kAffinity is invalid.
    ///
    static Result set(const I32 kPriority,
                      const Policy kPolicy,
                      const U8 kAffinity);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed Thread is uninitialized and invoking any of its
    /// methods returns an error.
    ///
    Thread();

    ///
    /// @brief Destructor. If the Thread was initialized, the destructor blocks
    /// until the thread terminates.
    ///
    ~Thread();

    ///
    /// @brief Waits for the thread to terminate.
    ///
    /// @param[out] kThreadRes  On success, if non-null, will be set to the
    ///                         thread return value.
    ///
    /// @retval SUCCESS       Successfully awaited thread.
    /// @retval E_THR_UNINIT  Thread is uninitialized.
    /// @retval E_THR_AWAIT   Failed to await thread. This usually indicates an
    ///                       error in the underlying syscall.
    ///
    Result await(Result* const kThreadRes);

    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread& operator=(Thread&&) = delete;

private:

    ///
    /// @brief Whether the thread is initialized.
    ///
    bool mInit;

#ifdef SF_PLATFORM_LINUX

    ///
    /// @brief Handle to underlying pthread.
    ///
    pthread_t mPthread;

    ///
    /// @brief Pthread wrapper arguments.
    ///
    struct PthreadWrapperArgs final
    {
        Function func; ///< Thread function for wrapper to run.
        void* args;    ///< Thread arguments.
    };

    ///
    /// @brief Wrapper arguments for this thread.
    ///
    PthreadWrapperArgs mWrapperArgs;

    ///
    /// @brief Pthread wrapper around the user-provided thread function.
    ///
    /// @param[in] kArgs  Pointer to PthreadWrapperArgs.
    ///
    /// @returns  Result of user-provided thread, reinterpreted as void*.
    ///
    static void* pthreadWrapper(void* kArgs);

    ///
    /// @brief Gets the pthread constant corresponding to a particular Policy.
    ///
    /// @param[in] kPolicy        Policy.
    /// @param[in] kPriority      Desired priority for policy.
    /// @param[in] kThreadPolicy  On success, set to pthread policy constant.
    ///
    /// @retval SUCCESS    Successfully got pthread policy.
    /// @retval E_THR_PRI  kPriority is invalid for the specified policy.
    /// @retval E_THR_POL  kPolicy is invalid.
    ///
    static Result getPthreadPolicy(const Policy kPolicy,
                                   const I32 kPriority,
                                   I32& kPthreadPolicy);

#endif
};

#endif
