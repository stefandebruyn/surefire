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
/// @file  sf/core/Diagnostic.hpp
/// @brief Diagnostic utilities.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_DIAG_HPP
#define SF_DIAG_HPP

#include "sf/core/Result.hpp"

///
/// @brief Namespace of diagnostic utilities.
///
namespace Diag
{
    ///
    /// @brief Prints a message if a result is an error code.
    ///
    /// @note Uses the PSL Console::printf implementation.
    ///
    /// @param[in] kRes  Result to evaluate.
    /// @param[in] kMsg  Message to print on error.
    ///
    void printOnError(const Result kRes, const char* const kMsg);

    ///
    /// @brief Prints a message and halts the program if a result is an error
    /// code.
    ///
    /// @note Uses the PSL Console::printf and System::exit implementations.
    ///
    /// @param[in] kRes  Result to evaluate.
    /// @param[in] kMsg  Message to print on error.
    ///
    void haltOnError(const Result kRes, const char* const kMsg);

    ///
    /// @brief "Error store conditional" - if a result is an error code, stores
    /// the error code to a reference if the reference is not already storing an
    /// error code.
    ///
    /// @param[in]  kRes      Result to evaluate.
    /// @param[out] kStorage  Reference to store error code in.
    ///
    inline void errsc(const Result kRes, Result& kStorage)
    {
        if ((kRes != SUCCESS) && (kStorage == SUCCESS))
        {
            kStorage = kRes;
        }
    }
}

#endif
