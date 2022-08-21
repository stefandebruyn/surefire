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
/// @file  sf/pal/utest/UTestConsole.cpp
/// @brief Unit tests for Console.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/Console.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

///
/// @brief Unit tests for Console.
///
TEST_GROUP(Console)
{
};

///
/// @test Console::printf() returns the correct number of printed characters.
///
TEST(Console, Printf)
{
    const I32 ret =
        Console::printf("Console print test %s %d %.2f", "foobar", 1522, 9.807);
    CHECK_EQUAL(35, ret);
}
