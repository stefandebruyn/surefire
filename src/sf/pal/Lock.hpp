////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/pal/Lock.hpp
/// @brief Platform-agnostic lock interface.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_LOCK_HPP
#define SF_LOCK_HPP

#include "sf/core/Result.hpp"

///
/// @brief Platform-agnmostic lock interface. This is intended for any
/// synchronization primitive that may be "acquired" and "released", though the
/// meaning of these operations is implementation-defined.
///
class ILock
{
public:

    ///
    /// @brief Acquires the lock.
    ///
    /// @post On success, current thread has acquired the lock.
    /// @post On error, current thread has not acquired the lock. The state of
    ///       the lock is unchanged.
    ///
    /// @retval SUCCESS  Successfully acquired lock.
    /// @retval [other]  Failed to acquire lock.
    ///
    virtual Result acquire() = 0;

    ///
    /// @brief Releases the lock.
    ///
    /// @post On success, current thread has released the lock.
    /// @post On error, current thread has not released the lock. The state of
    ///       the lock is unchanged.
    ///
    /// @retval SUCCESS  Successfully acquired lock.
    /// @retval [other]  Failed to acquire lock.
    ///
    virtual Result release() = 0;
};

#endif
