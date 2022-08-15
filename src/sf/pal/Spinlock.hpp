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
/// @file  sf/pal/Spinlock.hpp
/// @brief Platform-agnostic spinlock interface.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_SPINLOCK_HPP
#define SF_SPINLOCK_HPP

#ifdef SF_PLATFORM_LINUX
#    include <pthread.h>
#endif

#include "sf/core/Result.hpp"
#include "sf/pal/Lock.hpp"

///
/// @brief Spinlock synchronization primitive. Acquiring a spinlock means
/// busy-waiting in a loop while the lock is held by another thread.
///
/// @note Generally, it should be impossible for an error to occur while
/// initializing, acquiring, and releasing a spinlock, assuming correct
/// semantics. For this reason, other parts of the framework may use a spinlock
/// for short critical sections in scopes that may not surface errors (e.g.,
/// StateVector Element accesses).
///
class Spinlock final : public ILock
{
public:

    ///
    /// @brief Initializes a spinlock.
    ///
    /// @pre  kLock is uninitialized.
    /// @post On success, kLock is initialized and invoking methods on it may
    ///       succeed.
    /// @post On error, preconditions still hold.
    ///
    /// @param[in] kLock  Spinlock to initialize.
    ///
    static Result init(Spinlock& kLock);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed Spinlock is uninitialized and invoking any of its
    /// methods returns an error.
    ///
    Spinlock();

    ///
    /// @brief Destructor.
    ///
    /// @pre Spinlock is not held by any threads.
    ///
    ~Spinlock();

    ///
    /// @brief Acquires the spinlock. If another thread holds the spinlock, the
    /// calling thread will busy-wait until the spinlock is available. If the
    /// calling thread already holds the spinlock, the behavior is undefined.
    ///
    /// @note Linux: A thread which tries to acquire the spinlock while already
    /// holding it will deadlock.
    ///
    /// @retval SUCCESS       Successfully acquired spinlock.
    /// @retval E_SLK_UNINIT  Spinlock is not uninitialized.
    /// @retval E_SLK_ACQ     Failed to release spinlock. This usually indicates
    ///                       that the underlying platform API failed in some
    ///                       way.
    ///
    Result acquire() final override;

    ///
    /// @brief Releases the spinlock.
    ///
    /// @retval SUCCESS       Successfully released spinlock.
    /// @retval E_SLK_UNINIT  Spinlock is not uninitialized.
    /// @retval E_SLK_REL     Failed to release spinlock. This usually indicates
    ///                       the spinlock was not held by the calling thread,
    ///                       or the underlying platform API failed in some way.
    ///
    Result release() final override;

    Spinlock(const Spinlock&) = delete;
    Spinlock(Spinlock&&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;
    Spinlock& operator=(Spinlock&&) = delete;

private:

    ///
    /// @brief Whether the spinlock is initialized.
    ///
    bool mInit;

#ifdef SF_PLATFORM_LINUX

    ///
    /// @brief Underlying pthread spinlock.
    ///
    pthread_spinlock_t mLock;

#endif
};

#endif
