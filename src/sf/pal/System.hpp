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
/// @file  sf/pal/System.hpp
/// @brief Platform-agnostic interface for miscellaneous system functions.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_SYSTEM_HPP
#define SF_SYSTEM_HPP

#include "sf/core/BasicTypes.hpp"

///
/// @brief Platform-agnostic interface for miscellaneous system functions.
///
namespace System
{
    ///
    /// @brief Halts the program. Does not return.
    ///
    /// @note Linux: Exits the process with the specified exit status.
    /// @note Arduino: Enters an infinite loop. The exit status is disregarded.
    ///
    /// @param[in] kStatus  Exit status.
    ///
    void exit(const I32 kStatus);
}

#endif
