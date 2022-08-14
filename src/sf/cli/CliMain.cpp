////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/cli/CliMain.hpp
/// @brief CLI entry point.
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "sf/cli/CliUtil.hpp"
#include "sf/cli/StateMachineCommand.hpp"
#include "sf/cli/StateVectorCommand.hpp"
#include "sf/core/Assert.hpp"
#include "sf/core/BasicTypes.hpp"

///
/// @brief Prints the CLI help message.
///
static void printHelpMsg()
{
    // Top usage line.
    std::cout << Console::yellow << "usage" << Console::reset
              << ": sf <" << Console::cyan << "command" << Console::reset
              << "> [<" << Console::cyan << "args" << Console::reset
              << ">]\n\n";

    std::cout << "commands:\n";

    // State vector commands.
    std::cout << "  sv check <" << Console::cyan << "sv config path"
              << Console::reset << ">\n"
              << "    " << Console::yellow
              << "=> validate state vector config" << Console::reset << "\n";

    std::cout << "  sv autocode <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan << "autocode path"
              << Console::reset << "> <" << Console::cyan << "name"
              << Console::reset << "> [<" << Console::cyan << "regions"
              << Console::reset << ">]\n" << "    " << Console::yellow
              << "=> generate state vector autocode" << Console::reset << "\n";

    // State machine commands.
    std::cout << "  sm check <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan << "sm config path"
              << Console::reset << ">\n"
              << "    " << Console::yellow
              << "=> validate state machine config" << Console::reset << "\n";

    std::cout << "  sm autocode <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan << "sm config path"
              << Console::reset << "> <" << Console::cyan << "autocode path"
              << Console::reset << "> <" << Console::cyan << "name"
              << Console::reset << ">\n    " << Console::yellow
              << "=> generate state machine autocode" << Console::reset << "\n";

    std::cout << "  sm test <" << Console::cyan << "sv config path"
              << Console::reset << "> <" << Console::cyan
              << "sm config path" << Console::reset << "> <" << Console::cyan
              << "state script path" << Console::reset << ">\n"
              << "    " << Console::yellow
              << "=> run state script" << Console::reset << "\n";

    std::cout << std::flush;
}

///
/// @brief Entry point.
///
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
