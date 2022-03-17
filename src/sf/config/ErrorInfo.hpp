#ifndef SF_CONFIG_SOURCE_HPP
#define SF_CONFIG_SOURCE_HPP

#include <string>
#include <vector>

#include "sf/core/BasicTypes.hpp"

struct ErrorInfo final
{
    std::string text;

    std::string filePath;

    I32 lineNum;

    I32 colNum;

    std::string subtext;

    std::vector<std::string> lines;

    ErrorInfo();

    std::string prettifyError() const;
};

#endif
