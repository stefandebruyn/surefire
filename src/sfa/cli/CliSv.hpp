#ifndef SFA_CLI_SV_HPP
#define SFA_CLI_SV_HPP

#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>

#include "sfa/sup/StateVectorParser.hpp"

namespace Cli
{
    extern const std::string svHelpMsg;

    I32 sv(const std::vector<std::string> kArgs);
}

#endif
