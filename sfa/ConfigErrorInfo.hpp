#ifndef SFA_CONFIG_SOURCE_HPP
#define SFA_CONFIG_SOURCE_HPP

#include <vector>
#include <string>

#include "sfa/BasicTypes.hpp"

struct ConfigErrorInfo final
{
    std::string filePath;

    std::vector<std::string> lines;

    I32 lineNum;

    I32 colNum;

    std::string msg;

    ConfigErrorInfo();

    std::string prettifyError() const;
};

#endif
