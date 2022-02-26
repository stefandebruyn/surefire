#include <algorithm>

#include "sfa/sup/ConfigUtil.hpp"

const std::unordered_set<std::string> ConfigUtil::elemTypes =
{
    "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "F32", "F64", "bool"
};

const std::unordered_set<std::string> ConfigUtil::reserved =
{
    "STATE_VECTOR",
    "LOCAL",
    "IF",
    "ELSE",
    "NOT",
    "ENTRY",
    "STEP",
    "EXIT",
    "T",
    "G",
    "S"
};

bool ConfigUtil::isElementType(const std::string kType)
{
    return (std::find(elemTypes.begin(), elemTypes.end(), kType)
            != elemTypes.end());
}

bool ConfigUtil::isReserved(const std::string kStr)
{
    return (std::find(reserved.begin(), reserved.end(), kStr)
            != reserved.end());
}
