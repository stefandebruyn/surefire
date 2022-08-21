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
/// @file  sf/core/utest/UTestExpressionStats.cpp
/// @brief Unit tests for the MemOps namespace.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/MemOps.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

///
/// @brief Unit tests for the MemOps namespace.
///
TEST_GROUP(MemOps)
{
};

///
/// @test MemOps::strcmp() is correct for single-character strings.
///
TEST(MemOps, StrcmpSingleChars)
{
    CHECK_EQUAL(0, MemOps::strcmp("a", "a"));
    CHECK_EQUAL(('a' - 'A'), MemOps::strcmp("a", "A"));
    CHECK_EQUAL(('A' - 'a'), MemOps::strcmp("A", "a"));
}

///
/// @test MemOps::strcmp() is correct for multi-character strings.
///
TEST(MemOps, StrcmpStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp("foo bar baz", "foo bar baz"));
    CHECK_EQUAL(('b' - 'z'), MemOps::strcmp("foo bar baz", "foo zar baz"));
    CHECK_EQUAL(('z' - 'b'), MemOps::strcmp("foo zar baz", "foo bar baz"));
}

///
/// @test MemOps::strcmp() is correct for empty strings.
///
TEST(MemOps, StrcmpEmptyStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp("", ""));
    CHECK_EQUAL('f', MemOps::strcmp("foo", ""));
    CHECK_EQUAL((0 - 'f'), MemOps::strcmp("", "foo"));
}

///
/// @test MemOps::strcmp() treats null strings like empty strings.
///
TEST(MemOps, StrcmpNullStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp(nullptr, nullptr));
    CHECK_EQUAL(0, MemOps::strcmp("", nullptr));
    CHECK_EQUAL(0, MemOps::strcmp(nullptr, ""));
    CHECK_EQUAL('f', MemOps::strcmp("foo", nullptr));
    CHECK_EQUAL((0 - 'f'), MemOps::strcmp(nullptr, "foo"));
}

///
/// @test MemOps::memcpy() correctly copies 1 byte.
///
TEST(MemOps, MemcpyOneByte)
{
    U32 src = 0xB3;
    U32 dest = 0;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, &src, 1));
    CHECK_EQUAL(0xB3, src);
    CHECK_EQUAL(src, dest);
}

///
/// @test MemOps::memcpy() correctly copies multiple bytes.
///
TEST(MemOps, MemcpyMultipleBytes)
{
    U64 src = 0x77FA01B321;
    U64 dest = 0;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, &src, 5));
    CHECK_EQUAL(0x77FA01B321, src);
    CHECK_EQUAL(src, dest);
}

///
/// @test MemOps::memcpy() does nothing when the destination pointer is null.
///
TEST(MemOps, MemcpyDestNull)
{
    U32 src = 0xB3;
    POINTERS_EQUAL(nullptr, MemOps::memcpy(nullptr, &src, 4));
    CHECK_EQUAL(0xB3, src);
}

///
/// @test MemOps::memcpy() does nothing when the source pointer is null.
///
TEST(MemOps, MemcpySrcNull)
{
    U32 dest = 0xB3;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, nullptr, 4));
    CHECK_EQUAL(0xB3, dest);
}
