#include <iostream>

#include "sfa/core/BasicTypes.hpp"
#include "sfa/cli/SvCmd.hpp"
#include "sfa/cli/SmCmd.hpp"

static const std::string gHelpMsg = "idk";

I32 main(I32 kArgc, char* kArgv[])
{
    // Convert arguments into vector of string.
    std::vector<std::string> args(kArgc);
    for (std::size_t i = 0; i < args.size(); ++i)
    {
        args[i] = kArgv[i];
    }

    // Print help message when no arguments supplied.
    if (args.size() == 1)
    {
        std::cout << gHelpMsg << std::endl;
        return 0;
    }

    if (args[1] == "sv")
    {
        // State vector command.
        return Cli::sv(std::vector<std::string>(args.begin() + 2, args.end()));
    }
    else if (args[1] == "sm")
    {
        // State machine command.
        return Cli::sm(std::vector<std::string>(args.begin() + 2, args.end()));
    }
    else
    {
        std::cout << "error: unknown command `" << kArgv[1] << "`" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
