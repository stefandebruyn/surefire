#ifndef SF_CONFIG_UTIL_HPP
#define SF_CONFIG_UTIL_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"

namespace ConfigUtil
{
    void setError(ErrorInfo* const kErr,
                  const Token& kTokErr,
                  const String kText,
                  const String kSubtext);
}

#endif
