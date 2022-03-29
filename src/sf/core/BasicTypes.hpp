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
/// @file  sf/core/BasicTypes.hpp
/// @brief Typedefs for fixed-size, built-in types.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_BASIC_TYPES_HPP
#define SF_BASIC_TYPES_HPP

// This is the one header that the framework assumes is available. If it isn't,
// the user must provide it themselves or modify the typedefs in this file.
#include <stdint.h>

///
/// @brief Signed 8-bit integer.
///
typedef int8_t I8;
static_assert(sizeof(I8) == 1, "I8 is not 1 byte");

///
/// @brief Signed 16-bit integer.
///
typedef int16_t I16;
static_assert(sizeof(I16) == 2, "I16 is not 2 bytes");

///
/// @brief Signed 32-bit integer.
///
typedef int32_t I32;
static_assert(sizeof(I32) == 4, "I32 is not 4 bytes");

///
/// @brief Signed 64-bit integer.
///
typedef int64_t I64;
static_assert(sizeof(I64) == 8, "I64 is not 8 bytes");

///
/// @brief Unsigned 8-bit integer.
///
typedef uint8_t U8;
static_assert(sizeof(U8) == 1, "U8 is not 1 byte");

///
/// @brief Unsigned 16-bit integer.
///
typedef uint16_t U16;
static_assert(sizeof(U16) == 2, "U16 is not 2 bytes");

///
/// @brief Unsigned 32-bit integer.
///
typedef uint32_t U32;
static_assert(sizeof(U32) == 4, "U32 is not 4 bytes");

///
/// @brief Unsigned 64-bit integer.
///
typedef uint64_t U64;
static_assert(sizeof(U64) == 8, "U64 is not 8 bytes");

///
/// @brief Single-precision floating point.
///
typedef float F32;
static_assert(sizeof(F32) == 4, "F32 is not 4 bytes");

///
/// @brief Double-precision floating point.
///
/// @note This type may not actually be F64 on certain platforms that don't
/// support double precision. The type is still defined since disabling
/// framework code that uses F64 isn't convenient. In such cases, the user must
/// take care that the incorrect size of the type does not cause memory access
/// issues (e.g., by transmitting an F64 between applications on platforms with
/// different F64 sizes).
///
typedef double F64;
#ifndef SF_ALLOW_MISSIZED_F64
    // Only assert that F64 is 8 bytes on platforms where this is expected.
    static_assert(sizeof(F64) == 8, "F64 is not 8 bytes");
#endif

#endif
