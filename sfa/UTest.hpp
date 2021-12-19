#ifndef SFA_UTEST_HELPERS_HPP
#define SFA_UTEST_HELPERS_HPP

#include "sfa/Result.hpp"
#include "CppUTest/TestHarness.h"

#define CHECK_SUCCESS(kExpr)                                                   \
{                                                                              \
    const Result res = (kExpr);                                                \
    CHECK_EQUAL(SUCCESS, res);                                                 \
}

#endif
