#include "sfa/utest/UTest.hpp"

#if defined(SFA_UTEST_HAS_SUP)

SimpleString StringFrom(const Token& kTok)
{
    std::stringstream ss;
    ss << kTok;
    return SimpleString(ss.str().c_str());
}

#endif // defined(SFA_UTEST_HAS_SUP)
