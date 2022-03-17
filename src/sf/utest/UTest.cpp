#include "sf/utest/UTest.hpp"

#if defined(SF_UTEST_HAS_SUP)

SimpleString StringFrom(const Token& kTok)
{
    std::stringstream ss;
    ss << kTok;
    return SimpleString(ss.str().c_str());
}

#endif // defined(SF_UTEST_HAS_SUP)
