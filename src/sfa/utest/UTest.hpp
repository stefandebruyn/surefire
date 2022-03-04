#ifndef SFA_UTEST_HPP
#define SFA_UTEST_HPP

#ifdef __has_include
#    if __has_include("sfa/sup/ConfigTokenizer.hpp")
#        include <vector>
#        include <sstream>
#        include "sfa/sup/ConfigTokenizer.hpp"
#        define SFA_UTEST_HAS_SUP
#    endif
#endif

#include "sfa/core/Result.hpp"
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

#if defined(SFA_UTEST_HAS_SUP)

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

#endif // defined(SFA_UTEST_HAS_SUP)

#endif
