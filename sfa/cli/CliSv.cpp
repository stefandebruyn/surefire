#include "pal/Console.hpp"
#include "sfa/cli/CliSv.hpp"

const std::string Cli::svHelpMsg = "usage: sv <state vector config file path>";

I32 Cli::sv(const std::vector<std::string> kArgs)
{
    if (kArgs.size() == 0)
    {
        // No arguments to `sv`- print help message.
        std::cout << svHelpMsg << std::endl;
        return EXIT_SUCCESS;
    }

    if (kArgs[0] == "check")
    {
        // Check state vector config command.

        if (kArgs.size() == 1)
        {
            // No config file provided.
            std::cout << "error: `sv check` requires a file path" << std::endl;
            return EXIT_FAILURE;
        }

        // Attempt to parse config file.
        std::shared_ptr<StateVectorParser::Config> config = nullptr;
        ConfigErrorInfo err;
        const Result parseRes =
            StateVectorParser::parse(kArgs[1], config, &err);

        if (parseRes == SUCCESS)
        {
            // Config is valid.
            const StateVectorParser::Parse& parse = config->getParse();
            U32 totalElems = 0;
            U32 totalBytes = 0;

            for (const StateVectorParser::RegionParse& rgn : parse.regions)
            {
                // Compute region size in bytes.
                U32 regionBytes = 0;
                totalElems += rgn.elems.size();
                for (const StateVectorParser::ElementParse& elem : rgn.elems)
                {
                    regionBytes += elem.sizeBytes;
                }

                // Print region info.
                std::cout << "region `" << Console::cyan << rgn.plainName
                          << Console::reset << "`: " << Console::cyan
                          << rgn.elems.size() << Console::reset << " elements, "
                          << Console::cyan << regionBytes << Console::reset
                          << " B" << "\n";

                totalBytes += regionBytes;
            }

            // Print total state vector info.
            std::cout << "state vector: " << Console::cyan << totalElems
                      << Console::reset << " elements, " << Console::cyan
                      << totalBytes << Console::reset << " B" << std::endl;

            return EXIT_SUCCESS;
        }
        else
        {
            // Config is invalid.
            std::cout << err.prettifyError() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else
    {
        // Unknown argument after `sv`.
        std::cout << "error: unknown state vector command `" << kArgs[0] << "`"
                  << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
