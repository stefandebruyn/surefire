#ifndef SFA_UTEST_HPP
#define SFA_UTEST_HPP

#ifdef __has_include
#    if __has_include(<vector>)
#        include <vector>
#        define SFA_UTEST_HAS_VECTOR
#    endif
#    if __has_include(<sstream>)
#        include <sstream>
#        define SFA_UTEST_HAS_SSTREAM
#    endif
#endif

#include "sfa/Result.hpp"
#include "CppUTest/TestHarness.h"

#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    const Result _res = (kExpr);                                               \
    CHECK_EQUAL(SUCCESS, _res);                                                \
}

#if defined(SFA_UTEST_HAS_VECTOR) && defined(SFA_UTEST_HAS_SSTREAM)
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
#endif

#endif
