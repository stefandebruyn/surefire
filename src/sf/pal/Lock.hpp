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
    /// @post On SUCCESS, current thread has acquired the lock.
    /// @post On error, current thread has not acquired the lock. The lock is
    ///       unchanged.
    ///
    /// @retval SUCCESS  Successfully acquired lock.
    /// @retval [other]  Failed to acquire lock.
    ///
    virtual Result acquire() = 0;

    ///
    /// @brief Releases the lock.
    ///
    /// @post On SUCCESS, current thread has released the lock.
    /// @post On error, current thread has not released the lock. The lock is
    ///       unchanged.
    ///
    /// @retval SUCCESS  Successfully acquired lock.
    /// @retval [other]  Failed to acquire lock.
    ///
    virtual Result release() = 0;
};

#endif
