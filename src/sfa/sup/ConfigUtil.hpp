#ifndef SFA_CONFIG_UTIL_HPP
#define SFA_CONFIG_UTIL_HPP

#include <unordered_set>
#include <string>

namespace ConfigUtil
{
    extern const std::unordered_set<std::string> elemTypes;

    extern const std::unordered_set<std::string> reserved;

    bool isElementType(const std::string kType);

    bool isReserved(const std::string kStr);
}

#endif
