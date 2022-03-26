#include "sf/cli/CliUtil.hpp"
#include "sf/pal/Console.hpp"

std::ostream& Cli::error()
{
    return (std::cout << Console::red << "error" << Console::reset << ": ");
}
