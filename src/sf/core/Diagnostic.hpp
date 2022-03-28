////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/Diagnostic.hpp
/// @brief Diagnostic utilities.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_DIAG_HPP
#define SF_DIAG_HPP

#include "sf/core/Result.hpp"

///
/// @brief Namespace of diagnostic utility functions.
///
namespace Diag
{
    ///
    /// @brief Prints a message if a result is an error code.
    ///
    /// @note Uses the PSL `Console::printf` implementation.
    ///
    /// @param[in] kRes  Result to evaluate.
    /// @param[in] kMsg  Message to print on error.
    ///
    void printOnError(const Result kRes, const char* const kMsg);

    ///
    /// @brief Prints a message and halts the program if a result is an error
    /// code.
    ///
    /// @note Uses the PSL `Console::printf` and `System::exit` implementations.
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
