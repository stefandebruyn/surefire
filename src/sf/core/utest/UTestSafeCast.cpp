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
/// @file  sf/core/utest/UTestRegion.cpp
/// @brief Unit tests for ExprOpFuncs::safeCast().
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Expression.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that casting between two types with ExprOpFuncs::safeCast()
/// yields the expected value.
///
/// @param[in] TTo      Cast destination type name.
/// @param[in] TFrom    Cast source type name.
/// @param[in] kExpect  Expected value after cast.
/// @param[in] kVal     Value to cast.
///
#define CHECK_CAST(TTo, TFrom, kExpect, kVal)                                  \
{                                                                              \
    const TTo expect = kExpect;                                                \
    const TTo actual = ExprOpFuncs::safeCast<TTo, TFrom>(kVal);                \
    CHECK_EQUAL(expect, actual);                                               \
}

///
/// @brief Positive infinity constant.
///
static const F64 gInf = (1.0 / 0.0);

///
/// @brief Negative infinity constant.
///
static const F64 gNegInf = (-1.0 / 0.0);

///
/// @brief NaN constant.
///
static const F64 gNan = (0.0 / 0.0);

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for ExprOpFuncs::safeCast().
///
TEST_GROUP(SafeCast)
{
};

///
/// @test F64 is cast to I8 correctly.
///
TEST(SafeCast, F64ToI8)
{
    CHECK_CAST(I8, F64, 100, 100.0);
    CHECK_CAST(I8, F64, 100, 100.99);
    CHECK_CAST(I8, F64, -100, -100.0);
    CHECK_CAST(I8, F64, -100, -100.99);
    CHECK_CAST(I8, F64, 127, 128);
    CHECK_CAST(I8, F64, -128, -129);
    CHECK_CAST(I8, F64, 127, gInf);
    CHECK_CAST(I8, F64, -128, gNegInf);
    CHECK_CAST(I8, F64, 0, gNan);
}

///
/// @test F64 is cast to I16 correctly.
///
TEST(SafeCast, F64ToI16)
{
    CHECK_CAST(I16, F64, 100, 100.0);
    CHECK_CAST(I16, F64, 100, 100.99);
    CHECK_CAST(I16, F64, -100, -100.0);
    CHECK_CAST(I16, F64, -100, -100.99);
    CHECK_CAST(I16, F64, 32767, 32768.0);
    CHECK_CAST(I16, F64, -32768, -32769.0);
    CHECK_CAST(I16, F64, 32767, gInf);
    CHECK_CAST(I16, F64, -32768, gNegInf);
    CHECK_CAST(I16, F64, 0, gNan);
}

///
/// @test F64 is cast to I32 correctly.
///
TEST(SafeCast, F64ToI32)
{
    CHECK_CAST(I32, F64, 100, 100.0);
    CHECK_CAST(I32, F64, 100, 100.99);
    CHECK_CAST(I32, F64, -100, -100.0);
    CHECK_CAST(I32, F64, -100, -100.99);
    CHECK_CAST(I32, F64, 2147483647, 2147483648.0);
    CHECK_CAST(I32, F64, -2147483648, -2147483649.0);
    CHECK_CAST(I32, F64, 2147483647, gInf);
    CHECK_CAST(I32, F64, -2147483648, gNegInf);
    CHECK_CAST(I32, F64, 0, gNan);
}

///
/// @test F64 is cast to I64 correctly.
///
TEST(SafeCast, F64ToI64)
{
    // Spot check a few numbers.
    CHECK_CAST(I64, F64, 100, 100.0);
    CHECK_CAST(I64, F64, 100, 100.99);
    CHECK_CAST(I64, F64, -100, -100.0);
    CHECK_CAST(I64, F64, -100, -100.99);

    // Compute the first F64 value larger than I64 max by incrementing the
    // mantissa.
    const F64 i64MaxAsF64 = 9223372036854775807LL;
    union
    {
        F64 f64;
        U64 u64;
    } u;
    u.f64 = i64MaxAsF64;
    ++u.u64;

    // First F64 value larger than I64 max saturates at I64 max when cast.
    CHECK_CAST(I64, F64, 9223372036854775807LL, u.f64);

    // The I64 max gets slightly larger when cast to F64, so we expect it to
    // saturate at I64 max as well.
    CHECK_CAST(I64, F64, 9223372036854775807LL, i64MaxAsF64);

    // Compute the first F64 value smaller than I64 min by incrementing the
    // mantissa (not decrementing, since it affects the magnitude).
    const F64 i64MinAsF64 = (-9223372036854775807LL - 1);
    u.f64 = i64MinAsF64;
    ++u.u64;

    // First F64 value smaller than I64 min saturates at I64 min when cast.
    CHECK_CAST(I64, F64, (-9223372036854775807LL - 1), u.f64);

    // The I64 min can be exactly represented as F64, so we expect it to
    // saturate at I64 min as well.
    CHECK_CAST(I64, F64, (-9223372036854775807LL - 1), i64MinAsF64);

    // Check cases for special floating values.
    CHECK_CAST(I64, F64, 9223372036854775807LL, gInf);
    CHECK_CAST(I64, F64, (-9223372036854775807LL - 1), gNegInf);
    CHECK_CAST(I64, F64, 0, gNan);
}

///
/// @test F64 is cast to U8 correctly.
///
TEST(SafeCast, F64ToU8)
{
    CHECK_CAST(U8, F64, 100, 100.0);
    CHECK_CAST(U8, F64, 100, 100.99);
    CHECK_CAST(U8, F64, 255, 256.0);
    CHECK_CAST(U8, F64, 0, -1);
    CHECK_CAST(U8, F64, 255, gInf);
    CHECK_CAST(U8, F64, 0, gNegInf);
    CHECK_CAST(U8, F64, 0, gNan);
}

///
/// @test F64 is cast to U16 correctly.
///
TEST(SafeCast, F64ToU16)
{
    CHECK_CAST(U16, F64, 100, 100.0);
    CHECK_CAST(U16, F64, 100, 100.99);
    CHECK_CAST(U16, F64, 65535, 65536.0);
    CHECK_CAST(U16, F64, 0, -1);
    CHECK_CAST(U16, F64, 65535, gInf);
    CHECK_CAST(U16, F64, 0, gNegInf);
    CHECK_CAST(U16, F64, 0, gNan);
}

///
/// @test F64 is cast to U32 correctly.
///
TEST(SafeCast, F64ToU32)
{
    CHECK_CAST(U32, F64, 100, 100.0);
    CHECK_CAST(U32, F64, 100, 100.99);
    CHECK_CAST(U32, F64, 4294967295U, 4294967296.0);
    CHECK_CAST(U32, F64, 0, -1);
    CHECK_CAST(U32, F64, 4294967295U, gInf);
    CHECK_CAST(U32, F64, 0, gNegInf);
    CHECK_CAST(U32, F64, 0, gNan);
}

///
/// @test F64 is cast to U64 correctly.
///
TEST(SafeCast, F64ToU64)
{
    // Spot check a few values.
    CHECK_CAST(U64, F64, 100, 100.0);
    CHECK_CAST(U64, F64, 100, 100.99);

    // Compute the first F64 value larger than U64 max by incrementing the
    // mantissa.
    const F64 u64MaxAsF64 = 18446744073709551615ULL;
    union
    {
        F64 f64;
        U64 u64;
    } u;
    u.f64 = u64MaxAsF64;
    ++u.u64;

    // The first F64 value larger than U64 max saturates at U64 max when cast.
    CHECK_CAST(U64, F64, 18446744073709551615ULL, u.f64);

    // The U64 max gets slightly larger when cast to F64, so we expect it to
    // saturate at U64 max as well.
    CHECK_CAST(U64, F64, 18446744073709551615ULL, u64MaxAsF64);

    // Check some other special cases.
    CHECK_CAST(U64, F64, 0, -1);
    CHECK_CAST(U64, F64, 18446744073709551615ULL, gInf);
    CHECK_CAST(U64, F64, 0, gNegInf);
    CHECK_CAST(U64, F64, 0, gNan);
}

///
/// @test F64 is cast to F32 correctly.
///
TEST(SafeCast, F64ToF32)
{
    CHECK_CAST(F32, F64, static_cast<F32>(9.807), 9.807);
    CHECK_CAST(F32, F64, static_cast<F32>(-9.807), -9.807);
    CHECK_CAST(F32, F64, gInf, gInf);
    CHECK_CAST(F32, F64, gNegInf, gNegInf);
    CHECK_CAST(F32, F64, 0.0f, gNan);
}

///
/// @test F64 is cast to F64 correctly.
///
TEST(SafeCast, F64ToF64)
{
    CHECK_CAST(F64, F64, 9.807, 9.807);
    CHECK_CAST(F64, F64, -9.807, -9.807);
    CHECK_CAST(F64, F64, gInf, gInf);
    CHECK_CAST(F64, F64, gNegInf, gNegInf);
    CHECK_CAST(F64, F64, 0.0, gNan);
}

///
/// @test F64 is cast to bool correctly.
///
TEST(SafeCast, F64ToBool)
{
    CHECK_CAST(bool, F64, true, 9.807);
    CHECK_CAST(bool, F64, true, 1.0);
    CHECK_CAST(bool, F64, true, -1.0);
    CHECK_CAST(bool, F64, false, 0.0);
    CHECK_CAST(bool, F64, false, -0.0);
    CHECK_CAST(bool, F64, true, gInf);
    CHECK_CAST(bool, F64, true, gNegInf);
    CHECK_CAST(bool, F64, false, gNan);
}

///
/// @test I8 is cast to F64 correctly.
///
TEST(SafeCast, I8ToF64)
{
    CHECK_CAST(F64, I8, 100.0, 100);
    CHECK_CAST(F64, I8, -100.0, -100);
    CHECK_CAST(F64, I8, 127.0, Limits::max<I8>());
    CHECK_CAST(F64, I8, -128.0, Limits::min<I8>());
}

///
/// @test I16 is cast to F64 correctly.
///
TEST(SafeCast, I16ToF64)
{
    CHECK_CAST(F64, I16, 100.0, 100);
    CHECK_CAST(F64, I16, -100.0, -100);
    CHECK_CAST(F64, I16, 32767.0, Limits::max<I16>());
    CHECK_CAST(F64, I16, -32768.0, Limits::min<I16>());
}

///
/// @test I32 is cast to F64 correctly.
///
TEST(SafeCast, I32ToF64)
{
    CHECK_CAST(F64, I32, 100.0, 100);
    CHECK_CAST(F64, I32, -100.0, -100);
    CHECK_CAST(F64, I32, 2147483647.0, Limits::max<I32>());
    CHECK_CAST(F64, I32, -2147483648.0, Limits::min<I32>());
}

///
/// @test I64 is cast to F64 correctly.
///
TEST(SafeCast, I64ToF64)
{
    CHECK_CAST(F64, I64, 100.0, 100);
    CHECK_CAST(F64, I64, -100.0, -100);
    CHECK_CAST(F64, I64, 9223372036854775807.0, Limits::max<I64>());
    CHECK_CAST(F64, I64, -9223372036854775808.0, Limits::min<I64>());
}

///
/// @test U8 is cast to F64 correctly.
///
TEST(SafeCast, U8ToF64)
{
    CHECK_CAST(F64, U8, 100.0, 100);
    CHECK_CAST(F64, U8, 255.0, Limits::max<U8>());
    CHECK_CAST(F64, U8, 0.0, Limits::min<U8>());
}

///
/// @test U16 is cast to F64 correctly.
///
TEST(SafeCast, U16ToF64)
{
    CHECK_CAST(F64, U16, 100.0, 100);
    CHECK_CAST(F64, U16, 65535.0, Limits::max<U16>());
    CHECK_CAST(F64, U16, 0.0, Limits::min<U16>());
}

///
/// @test U32 is cast to F64 correctly.
///
TEST(SafeCast, U32ToF64)
{
    CHECK_CAST(F64, U32, 100.0, 100);
    CHECK_CAST(F64, U32, 4294967295.0, Limits::max<U32>());
    CHECK_CAST(F64, U32, 0.0, Limits::min<U32>());
}

///
/// @test U64 is cast to F64 correctly.
///
TEST(SafeCast, U64ToF64)
{
    CHECK_CAST(F64, U64, 100.0, 100);
    // Note that the expected F64 value here is not exactly the U64 max; a loss
    // of precision in the cast is expected.
    CHECK_CAST(F64, U64, 18446744073709551616.0, Limits::max<U64>());
    CHECK_CAST(F64, U64, 0.0, Limits::min<U64>());
}

///
/// @test F32 is cast to F64 correctly.
///
TEST(SafeCast, F32ToF64)
{
    CHECK_CAST(F64, F32, static_cast<F64>(9.807f), 9.807f);
    CHECK_CAST(F64, F32, static_cast<F64>(-9.807f), -9.807f);
    CHECK_CAST(F64, F32, gInf, gInf);
    CHECK_CAST(F64, F32, gNegInf, gNegInf);
    CHECK_CAST(F64, F32, 0.0, gNan);
}

///
/// @test Bool is cast to F64 correctly.
///
TEST(SafeCast, BoolToF64)
{
    CHECK_CAST(F64, bool, 1.0, true);
    CHECK_CAST(F64, bool, 0.0, false);

    // Can't trick the cast by passing a bool with a value other than 0 and 1
    // underlaying it.
    U8 x = 0xFF;
    bool y = *reinterpret_cast<bool*>(&x);
    CHECK_CAST(F64, bool, 1.0, y);
}
