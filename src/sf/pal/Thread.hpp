////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Built in Austin, Texas at the University of Texas at Austin.
/// Surefire is open-source under the Apache License 2.0 - a copy of the license
/// may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
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
