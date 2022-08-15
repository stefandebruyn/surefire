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
/// @file  sf/core/utest/UTestAssert.cpp
/// @brief Unit tests for assert macros.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Assert.hpp"
#include "sf/core/MemOps.hpp"
#include "sf/pal/Console.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Function which asserts the value of a bool argument.
///
static Result foo(const bool kFail)
{
    SF_SAFE_ASSERT(kFail);

    return SUCCESS;
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for assert macros.
///
TEST_GROUP(Assert)
{
    void teardown()
    {
        // Reset assert fail site.
        Assert::failFile = nullptr;
        Assert::failLineNum = -1;
    }
};

///
/// @test SF_SAFE_ASSERT fails when expected and saves the assert fail site
/// if configured.
///
TEST(Assert, SafeAssert)
{
    // No assert fail site.
    CHECK_TRUE(Assert::failFile == nullptr);
    CHECK_EQUAL(-1, Assert::failLineNum);

    // Assert pass case.
    CHECK_SUCCESS(foo(true));

    // Assert fail case.
    CHECK_ERROR(E_ASSERT, foo(false));

#ifdef SF_SAFE_ASSERT_SAVES_FAIL_SITE
    // Assert fail site was saved since the appropriate symbol is defined.
    Console::printf(" Note: assert fail site saving is enabled");
    CHECK_TRUE(Assert::failFile != nullptr);
    CHECK_TRUE(Assert::failLineNum != -1);
#else
    // Assert fail site still not saved since the symbol is not defined.
    Console::printf(" Note: assert fail site saving is disabled");
    CHECK_TRUE(Assert::failFile == nullptr);
    CHECK_EQUAL(-1, Assert::failLineNum);
#endif
}

///
/// @brief SF_ASSERT does nothing when the assert passes, and nothing when the
/// assert fails but asserts are disabled.
///
TEST(Assert, UnsafeAssert)
{
    // Asserting true never halts.
    SF_ASSERT(true);

#ifndef SF_ENABLE_ASSERTS
    // Asserting false does not halt when asserts are not enabled.
    Console::printf(" Note: asserts are disabled");
    SF_ASSERT(false);
#else
    Console::printf(" Note: asserts are enabled");
#endif
}
