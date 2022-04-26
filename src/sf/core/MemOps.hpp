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
/// @file  sf/core/MemOps.hpp
/// @brief Memory utilities.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_MEM_OPS_HPP
#define SF_MEM_OPS_HPP

#include "sf/core/BasicTypes.hpp"

///
/// @brief Namespace of memory utilities.
///
namespace MemOps
{
    ///
    /// @brief Compares two strings. Equivalent to the libc function of the same
    /// name, but safely handles null pointers. Null pointers are treated like
    /// empty strings.
    ///
    /// @param[in] kA  First string.
    /// @param[in] kB  Second string.
    ///
    /// @return Value <0 if the first string comes before the second, value >0
    /// if the first string comes after the second, and 0 if the two strings are
    /// equal.
    ///
    I32 strcmp(const char* kA, const char* kB);

    ///
    /// @brief Copies one region of memory to another. Equivalent to the libc
    /// function of the same name, but safely handles null pointers. If the
    /// source or destination pointer is null, the function has no effect.
    ///
    /// @note The source and destination regions must not overlap for a correct
    /// result.
    ///
    /// @param[in] kDest   Destination address.
    /// @param[in] kSrc    Source address.
    /// @param[in] kBytes  Number of bytes to copy from source to destination.
    ///
    /// @return Destination address.
    ///
    void* memcpy(void* kDest, const void* const kSrc, const U32 kBytes);
}

#endif
