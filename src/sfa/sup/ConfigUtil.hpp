#ifndef SFA_CONFIG_UTIL_HPP
#define SFA_CONFIG_UTIL_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "sfa/core/Element.hpp"
#include "sfa/sup/EnumHash.hpp"
#include "sfa/sup/TokenIterator.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"

namespace ConfigUtil
{
    struct ElementTypeInfo final
    {
        ElementType enumVal;
        std::string name;
        std::unordered_set<std::string> aliases;
        U32 sizeBytes;
        bool arithmetic;
        bool fp;
        bool sign;
    };

    extern const std::unordered_map<std::string, ElementTypeInfo>
        typeInfoFromName;

    extern const std::unordered_map<ElementType, ElementTypeInfo, EnumHash>
        typeInfoFromEnum;

    extern const std::unordered_set<std::string> reserved;

    void setError(ConfigErrorInfo* const kConfigErr,
                  const Token& kTokErr,
                  const std::string kText,
                  const std::string kSubtext);

    bool checkEof(const TokenIterator& kIt,
                  const Token& kTokLast,
                  const std::string kErrText,
                  ConfigErrorInfo* const kConfigErr);
}

#endif
