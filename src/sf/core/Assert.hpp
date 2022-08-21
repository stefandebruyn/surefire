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
/// @file  sf/core/Assert.hpp
/// @brief Assert macros.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ASSERT_HPP
#define SF_ASSERT_HPP

#include "sf/pal/Console.hpp"
#include "sf/pal/System.hpp"

///
/// @brief Macro for disabling code coverage.
///
#define SF_DISABLE_CODE_COV // foo

///
/// @brief Macro for enabling code coverage.
///
#define SF_ENABLE_CODE_COV // bar

#ifdef SF_ENABLE_ASSERTS
///
/// @brief "Unsafe" assert macro which halts the program on failure. Unsafe
/// asserts are only made if SF_ENABLE_ASSERTS was defined at compile time,
/// otherwise the asserts have no effect and disappear from code entirely.
/// Accordingly, assert expressions should not have side effects.
///
/// @note Uses the PSL Console::printf and System::exit implementations.
///
/// @remark Unsafe asserts are only used as a debugging tool in contexts from
/// which surfacing error codes is impossible.
///
/// @param[in] kExpr  Expression to assert. If untrue, program halts.
///
#define SF_ASSERT(kExpr)                                                       \
SF_DISABLE_CODE_COV;                                                           \
do                                                                             \
{                                                                              \
    const bool _res = (kExpr);                                                 \
    if (_res != true)                                                          \
    {                                                                          \
        ::Sf::Console::printf("\nAssertion failed at %s:%d:\n    %s\n",        \
                        __FILE__, __LINE__, #kExpr);                           \
        ::Sf::System::exit(1);                                                 \
    }                                                                          \
} while (false);                                                               \
SF_ENABLE_CODE_COV;
#else
#    define SF_ASSERT(kExpr)
#endif

#ifdef SF_SAFE_ASSERT_SAVES_FAIL_SITE
///
/// @brief Macro used to save the location of a failing assert in code to global
/// variables in the Assert namespace. If SF_SAFE_ASSERT_SAVES_FAIL_SITE was not
/// defined at compile time, the macro has no effect.
///
#define SF_SAVE_FAIL_SITE                                                      \
::Sf::Assert::failFile = __FILE__;                                             \
::Sf::Assert::failLineNum = __LINE__;
#else
#    define SF_SAVE_FAIL_SITE
#endif

///
/// @brief "Safe" assert macro which returns an error on failure instead of
/// halting the program.
///
/// This macro should only be used within functions that return a Result. If
/// SF_SAFE_ASSERT_SAVES_FAIL_SITE was defined at compile time, the macro saves
/// the location of the failing assert in Assert::failFile and
/// Assert::failLineNum. Opting not to save fail sites may be preferable when
/// the user is not debugging failed asserts, and the extra static data
/// generated by the __FILE__ and __LINE__ macros is undesirable.
///
/// @remark Safe asserts are commonly used to assert invariants and guard
/// pointer dereferences in framework code. Failed assertions are intended to
/// surface bugs in the framework and not mistakes by the user.
///
/// @param[in] kExpr  Expression to assert. If untrue, returns E_ASSERT from the
///                   current function.
///
#define SF_SAFE_ASSERT(kExpr)                                                  \
SF_DISABLE_CODE_COV;                                                           \
do                                                                             \
{                                                                              \
    const bool _res = (kExpr);                                                 \
    if (_res != true)                                                          \
    {                                                                          \
        /* Save fail site (if the appropriate symbol is defined). */           \
        SF_SAVE_FAIL_SITE;                                                     \
                                                                               \
        return E_ASSERT;                                                       \
    }                                                                          \
} while (false);                                                               \
SF_ENABLE_CODE_COV;

namespace Sf
{

///
/// @brief Namespace of global data for debugging asserts.
///
namespace Assert
{
    ///
    /// @brief Name of file containing last failed assert, or null if no
    /// failure.
    ///
    extern const char* failFile;

    ///
    /// @brief Line number of last failed assert, or -1 if no failure.
    ///
    extern I32 failLineNum;
}

} // namespace Sf

#endif
