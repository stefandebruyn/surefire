#ifndef SFA_CONFIG_SOURCE_HPP
#define SFA_CONFIG_SOURCE_HPP

#include <vector>
#include <string>

#include "sfa/BasicTypes.hpp"

struct ConfigErrorInfo final
{
    ConfigErrorInfo();

    I32 lineNum;

    I32 colNum;

    std::string msg;
};

struct ConfigInfo final
{
    std::string filePath;

    std::vector<std::string> lines;

    ConfigErrorInfo error;

    std::string prettifyError() const;
};

#endif
