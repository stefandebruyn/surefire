#ifndef SFA_CONFIG_UTIL_HPP
#define SFA_CONFIG_UTIL_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "sfa/core/Element.hpp"
#include "sfa/sup/EnumHash.hpp"

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
}

#endif
