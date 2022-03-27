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

#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif

#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    const Result _res = (kExpr);                                               \
    CHECK_EQUAL(SUCCESS, _res);                                                \
}

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

#define TOKENIZE(kStr)                                                         \
    Vec<Token> toks;                                                           \
    {                                                                          \
        std::stringstream _ss(kStr);                                           \
        CHECK_SUCCESS(Tokenizer::tokenize(_ss, toks, nullptr));                \
    }                                                                          \
    TokenIterator it(toks.begin(), toks.end());

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

SimpleString StringFrom(const Token& kTok);

#endif // defined(SF_UTEST_HAS_CONFIG)

#endif
