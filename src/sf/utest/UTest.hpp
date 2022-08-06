////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/utest/UTest.hpp
/// @brief Unit test helpers. This should be the last include at the top of
///        every unit test file.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_UTEST_HPP
#define SF_UTEST_HPP

#ifdef __has_include
#    if __has_include("sf/config/Tokenizer.hpp")
#        include <sstream>
#        include <vector>
#        include "sf/config/TokenIterator.hpp"
#        include "sf/config/Tokenizer.hpp"
#        define SF_UTEST_HAS_CONFIG
#    endif
#endif

#include "sf/core/Assert.hpp"
#include "sf/core/Result.hpp"
#include "sf/pal/Console.hpp"

// Must be the last include.
#include "CppUTest/TestHarness.h"

// Find the path separator appropriate for the current platform.
#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif

///
/// @brief Checks that an expression evaluates to SUCCESS.
///
/// @param[in] kExpr  Expression to evaluate.
///
#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    const Result _res = (kExpr);                                               \
    CHECK_EQUAL(SUCCESS, _res);                                                \
}

///
/// @brief Checks that an expression evaluates to the specified error code.
///
/// @param[in] kErr   Expected error code.
/// @param[in] kExpr  Expression to evaluate.
///
#define CHECK_ERROR(kErr, kExpr)                                               \
{                                                                              \
    const Result _res = (kExpr);                                               \
    if ((_res == E_ASSERT) && (Assert::failFile != nullptr))                   \
    {                                                                          \
        Console::printf("\n`E_ASSERT` originating at %s:%d\n",                 \
                        Assert::failFile,                                      \
                        Assert::failLineNum);                                  \
    }                                                                          \
    CHECK_EQUAL(kErr, _res);                                                   \
}

#if defined(SF_UTEST_HAS_CONFIG)

///
/// @brief Tokenizes a string into a `Vec<Token> toks` and `TokenIterator it` in
/// the current scope.
///
/// @param[in] kStr  String to tokenize.
///
#define TOKENIZE(kStr)                                                         \
    Vec<Token> toks;                                                           \
    {                                                                          \
        std::stringstream _ss(kStr);                                           \
        CHECK_SUCCESS(Tokenizer::tokenize(_ss, toks, nullptr));                \
    }                                                                          \
    TokenIterator it(toks.begin(), toks.end());

///
/// @brief Converts a vector to a string, so that vectors can be passed to more
/// CppUTest macros.
///
/// @param[in] kVec  Vector to convert.
///
/// @returns Vector as string.
///
template<typename T>
SimpleString StringFrom(const Vec<T>& kVec)
{
    std::stringstream ss;
    ss << "[";
    if (kVec.size() > 0)
    {
        ss << kVec[0];
        for (std::size_t i = 1; i < kVec.size(); ++i)
        {
            ss << ", " << kVec[i];
        }
    }
    ss << "]";
    return SimpleString(ss.str().c_str());
}

///
/// @brief Converts a token to a string, so that tokens can be passed to more
/// CppUTest macros.
///
/// @param[in] kTok  Token to convert.
///
/// @returns Token as string.
///
SimpleString StringFrom(const Token& kTok);

#endif // defined(SF_UTEST_HAS_CONFIG)

#endif
