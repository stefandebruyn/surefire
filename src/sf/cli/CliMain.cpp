#include <iostream>

#include "sf/cli/SmCmd.hpp"
#include "sf/cli/SvCmd.hpp"
#include "sf/core/BasicTypes.hpp"

static const String gHelpMsg = "idk";

I32 main(I32 kArgc, char* kArgv[])
{
    // Convert arguments into vector of string.
    Vec<String> args(kArgc);
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
        return Cli::sv(Vec<String>(args.begin() + 2, args.end()));
    }
    else if (args[1] == "sm")
    {
        // State machine command.
        return Cli::sm(Vec<String>(args.begin() + 2, args.end()));
    }
    else
    {
        std::cout << "error: unknown command `" << kArgv[1] << "`" << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
