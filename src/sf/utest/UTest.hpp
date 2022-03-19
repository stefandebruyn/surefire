#ifndef SF_UTEST_HPP
#define SF_UTEST_HPP

#ifdef __has_include
#    if __has_include("sf/config/Tokenizer.hpp")
#        include <sstream>
#        include <vector>
#        include "sf/config/TokenIterator.hpp"
#        include "sf/config/Tokenizer.hpp"
#        define SF_UTEST_HAS_SUP
#    endif
#endif

#include "sf/core/Result.hpp"

// Must be the last include.
#include "CppUTest/TestHarness.h"

#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    const Result _res = (kExpr);                                               \
    CHECK_EQUAL(SUCCESS, _res);                                                \
}

#define CHECK_ERROR(kErr, kExpr)                                               \
{                                                                              \
    const Result _res = (kExpr);                                               \
    CHECK_EQUAL(kErr, _res);                                                   \
}

#if defined(SF_UTEST_HAS_SUP)

#define TOKENIZE(kStr)                                                         \
    std::vector<Token> toks;                                                   \
    {                                                                          \
        std::stringstream _ss(kStr);                                           \
        CHECK_SUCCESS(Tokenizer::tokenize(_ss, toks, nullptr));                \
    }                                                                          \
    TokenIterator it(toks.begin(), toks.end());

template<typename T>
SimpleString StringFrom(const std::vector<T>& kVec)
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

#endif // defined(SF_UTEST_HAS_SUP)

#endif