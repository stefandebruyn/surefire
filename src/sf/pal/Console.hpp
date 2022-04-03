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
/// @file  sf/pal/Console.hpp
/// @brief Platform-agnostic interface for printing to the system console.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_CONSOLE_HPP
#define SF_CONSOLE_HPP

#include "sf/core/BasicTypes.hpp"

///
/// @brief Platform-agnostic interface for printing to the system console.
///
namespace Console
{
    ///
    /// @brief Format code for red text.
    ///
    extern const char* const red;

    ///
    /// @brief Format code for yellow text.
    ///
    extern const char* const yellow;

    ///
    /// @brief Format code for green text.
    ///
    extern const char* const green;

    ///
    /// @brief Format code for cyan text.
    ///
    extern const char* const cyan;

    ///
    /// @brief Format code for resetting the console text style.
    ///
    extern const char* const reset;

    ///
    /// @brief Prints to the system console with equivalent syntax and semantics
    /// to the libc printf() function.
    ///
    /// @warning Passing more optional arguments than there are specifiers in
    /// the format string has undefined behavior. Using incorrect specifiers in
    /// the format string also has undefined behavior.
    ///
    /// @note Linux: Prints to stdout.
    /// @note Arduino: Prints to Serial.
    ///
    /// @param[in] kFmt  String to print, with optional format specifiers.
    /// @param[in] ...   Optional values to format into the format string.
    ///
    /// @return Number of characters printed.
    ///
    I32 printf(const char* const kFmt, ...);
}

#endif
