#ifndef SFA_CONFIG_SOURCE_HPP
#define SFA_CONFIG_SOURCE_HPP

#include <vector>
#include <string>

#include "sfa/core/BasicTypes.hpp"

struct ConfigErrorInfo final
{
    std::string text;

    std::string filePath;

    I32 lineNum;

    I32 colNum;

    std::string subtext;

    std::vector<std::string> lines;

    ConfigErrorInfo();

    std::string prettifyError() const;
};

#endif
