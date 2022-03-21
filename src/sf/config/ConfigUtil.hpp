#ifndef SF_CONFIG_UTIL_HPP
#define SF_CONFIG_UTIL_HPP

#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"

namespace ConfigUtil
{
    extern const Set<String> reserved; // is this being used?????????//

    void setError(ErrorInfo* const kErr,
                  const Token& kTokErr,
                  const String kText,
                  const String kSubtext);

    bool checkEof(const TokenIterator& kIt,
                  const Token& kTokLast,
                  const String kErrText,
                  ErrorInfo* const kErr);
}

#endif
