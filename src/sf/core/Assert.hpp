#ifndef SF_ASSERT_HPP
#define SF_ASSERT_HPP

#include "sf/pal/Console.hpp"
#include "sf/pal/System.hpp"

#define SF_DISABLE_CODE_COV // foo

#define SF_ENABLE_CODE_COV // bar

#ifdef SF_ENABLE_ASSERTS
#    define SF_ASSERT(kExpr)                                                   \
SF_DISABLE_CODE_COV;                                                           \
do                                                                             \
{                                                                              \
    const bool _res = (kExpr);                                                 \
    if (_res != true)                                                          \
    {                                                                          \
        Console::printf("\nAssertion failed at %s:%d:\n    %s\n",              \
                        __FILE__, __LINE__, #kExpr);                           \
        System::exit(1);                                                       \
    }                                                                          \
} while (false);                                                               \
SF_ENABLE_CODE_COV;
#else
#    define SF_ASSERT(kExpr)
#endif

#ifdef SF_SAFE_ASSERT_SAVES_FAIL_SITE
#    define SF_SAVE_FAIL_SITE                                                  \
::Assert::failFile = __FILE__;                                                 \
::Assert::failLineNum = __LINE__;
#else
#    define SF_SAVE_FAIL_SITE
#endif

#define SF_SAFE_ASSERT_CLEAN(kExpr, kCleanupCode)                              \
SF_DISABLE_CODE_COV;                                                           \
do                                                                             \
{                                                                              \
    const bool _res = (kExpr);                                                 \
    if (_res != true)                                                          \
    {                                                                          \
        /* Assert failed- Run cleanup code. */                                 \
        kCleanupCode;                                                          \
                                                                               \
        /* Save fail site (if the appropriate symbol is defined). */           \
        SF_SAVE_FAIL_SITE;                                                     \
                                                                               \
        return E_ASSERT;                                                       \
    }                                                                          \
} while (false);                                                               \
SF_ENABLE_CODE_COV;

#define SF_SAFE_ASSERT(kExpr) SF_SAFE_ASSERT_CLEAN(kExpr, {})

namespace Assert
{
    extern const char* failFile;

    extern I32 failLineNum;
}

#endif
