#include <iostream>

#include "sf/cli/CliUtil.hpp"
#include "sf/cli/StateMachineCommand.hpp"
#include "sf/cli/StateVectorCommand.hpp"
#include "sf/core/Assert.hpp"
#include "sf/core/BasicTypes.hpp"

static void printHelpMsg()
{
    // Top usage line.
    std::cout << Console::yellow << "usage" << Console::reset
              << ": sf <" << Console::cyan << "command" << Console::reset
              << "> [<" << Console::cyan << "args" << Console::reset
              << ">]\n\n";

    std::cout << "commands:\n";

    // State vector commands.
    std::cout << "    sv check <" << Console::cyan << "sv config path"
              << Console::reset << ">\n"
              << "        " << Console::yellow
              << "=> validate state vector config" << Console::reset << "\n";

    std::cout << "    sv autocode <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan << "autocode path"
              << Console::reset << ">\n"
              << "        " << Console::yellow
              << "=> generate state vector autocode" << Console::reset << "\n";

    // State machine commands.
    std::cout << "    sm check <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan << "sm config path"
              << Console::reset << ">\n"
              << "        " << Console::yellow
              << "=> validate state machine config" << Console::reset << "\n";

    std::cout << "    sm autocode <" << Console::cyan << "sm config path"
              << Console::reset << "> <" << Console::cyan
              << "autocode output path" << Console::reset << ">\n"
              << "        " << Console::yellow
              << "=> generate state machine autocode" << Console::reset << "\n";

    std::cout << "    sm test <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan
              << "sm config path" << Console::reset << "> <" << Console::cyan
              << "state script path" << Console::reset << ">\n"
              << "        " << Console::yellow
              << "=> run state script" << Console::reset << "\n";

    std::cout << std::flush;
}

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
        printHelpMsg();
        return EXIT_SUCCESS;
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

    // If we got this far, command was not recognized.
    Cli::error() << "unknown command `" << args[1] << "`" << std::endl;
    return EXIT_FAILURE;
}
