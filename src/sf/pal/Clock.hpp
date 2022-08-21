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
/// @file  sf/pal/Clock.hpp
/// @brief Platform-agnostic interface for accessing the system clock and other
///        timekeeping utilities.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_CLOCK_HPP
#define SF_CLOCK_HPP

#include "sf/core/BasicTypes.hpp"

namespace Sf
{

///
/// @brief Platform-agnostic interface for accessing the system clock and other
/// timekeeping utilities.
///
namespace Clock
{
    ///
    /// @brief Number of nanoseconds in a second.
    ///
    constexpr U64 NS_IN_S = 1000000000;

    ///
    /// @brief Number of microseconds in a second.
    ///
    constexpr U64 US_IN_S = 1000000;

    ///
    /// @brief Number of milliseconds in a second.
    ///
    constexpr U64 MS_IN_S = 1000;

    ///
    /// @brief Value reserved by the framework to represent the absence of a
    /// time value. This is the largest time value possible at ~584 years (in
    /// nanoseconds), so it should never be produced by the clock.
    ///
    constexpr U64 NO_TIME = 0xFFFFFFFFFFFFFFFF;

    ///
    /// @brief Gets the system clock time in nanoseconds.
    ///
    /// @warning Linux: The real-time system clock is used so that time values
    /// reflect adjustments made by sync protocols like NTP. This means that
    /// time may not be monotonic if the real-time clock is adjusted between
    /// calls to Clock::nanoTime(). Care should also be taken that the system
    /// does not automatically adjust the real-time clock, e.g., for Daylight
    /// Saving Time.
    ///
    /// @note Linux: The Linux implementation of this function uses
    /// clock_gettime(). Since Clock::nanoTime() cannot surface errors, the
    /// return value of clock_gettime() is disregarded. Errors are not expected;
    /// EFAULT cannot occur since the timespec pointer passed to the function
    /// is always valid, and EINVAL cannot occur since CLOCK_REALTIME should
    /// always be a valid clock ID. If by some chance clock_gettime() does fail,
    /// Clock::nanoTime() returns 0.
    ///
    /// @return System time in nanoseconds.
    ///
    U64 nanoTime();

    ///
    /// @brief Spinwaits until some number of nanoseconds have passed according
    /// to the system clock.
    ///
    /// @note This will usually spinwait slightly longer than specified due to
    /// the overhead of the Clock::nanoTime() function.
    ///
    /// @param[in] kNs  Number of nanoseconds to spinwait for.
    ///
    inline void spinWait(const U64 kNs)
    {
        const U64 startNs = nanoTime();
        while ((nanoTime() - startNs) < kNs);
    }
}

} // namespace Sf

#endif
