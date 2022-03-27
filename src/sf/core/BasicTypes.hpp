#ifndef SF_BASIC_TYPES_HPP
#define SF_BASIC_TYPES_HPP

// This is the one header that the framework assumes is available. If it isn't
// the user must provide it themselves or modify the typedefs in this file.
#include <stdint.h>

typedef int8_t I8;
static_assert(sizeof(I8) == 1, "I8 is not 1 byte");

typedef int16_t I16;
static_assert(sizeof(I16) == 2, "I16 is not 2 bytes");

typedef int32_t I32;
static_assert(sizeof(I32) == 4, "I32 is not 4 bytes");

typedef int64_t I64;
static_assert(sizeof(I64) == 8, "I64 is not 8 bytes");

typedef uint8_t U8;
static_assert(sizeof(U8) == 1, "U8 is not 1 byte");

typedef uint16_t U16;
static_assert(sizeof(U16) == 2, "U16 is not 2 bytes");

typedef uint32_t U32;
static_assert(sizeof(U32) == 4, "U32 is not 4 bytes");

typedef uint64_t U64;
static_assert(sizeof(U64) == 8, "U64 is not 8 bytes");

typedef float F32;
static_assert(sizeof(F32) == 4, "F32 is not 4 bytes");

typedef double F64;
#ifndef SF_ALLOW_MISSIZED_F64
    // Only assert that `F64` is 8 bytes on platforms where this is expected.
    static_assert(sizeof(F64) == 8, "F64 is not 8 bytes");
#endif

#endif
