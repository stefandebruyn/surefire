#include <fstream>

#include "sf/cli/SvCmd.hpp"
#include "sf/config/ConfigUtil.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Console.hpp"

I32 Cli::sv(const Vec<String> kArgs)
{
    SF_ASSERT(kArgs.size() > 0);

    if (kArgs[0] == "check")
    {
        // Check state vector config command.

        SF_ASSERT(kArgs.size() == 2);

        // Compile state vector.
        Ref<StateVectorCompiler::Assembly> svAsm;
        ErrorInfo err;
        const Result res = StateVectorCompiler::compile(kArgs[1], svAsm, &err);

        if (res != SUCCESS)
        {
            // State vector config is invalid.
            std::cout << err.prettifyError() << std::endl;
            return EXIT_FAILURE;
        }

        // Config is valid; print some info about it.
        const StateVectorParser::Parse& parse = svAsm->getParse();
        U32 totalElems = 0;
        U32 totalBytes = 0;

        for (const StateVectorParser::RegionParse& region : parse.regions)
        {
            // Compute region size in bytes.
            U32 regionBytes = 0;
            totalElems += region.elems.size();
            for (const StateVectorParser::ElementParse& elem : region.elems)
            {
                SF_ASSERT(elem.tokType.typeInfo != nullptr);
                regionBytes += elem.tokType.typeInfo->sizeBytes;
            }

            // Print region info.
            std::cout << "region `" << Console::cyan << region.plainName
                        << Console::reset << "`: " << Console::cyan
                        << region.elems.size() << Console::reset
                        << " elements, " << Console::cyan << regionBytes
                        << Console::reset << " B" << "\n";

            totalBytes += regionBytes;
        }

        // Print total state vector info.
        std::cout << "state vector: " << Console::cyan << totalElems
                    << Console::reset << " elements, " << Console::cyan
                    << totalBytes << Console::reset << " B" << std::endl;
    }
    else
    {
        SF_ASSERT(false);
    }

    return EXIT_SUCCESS;
}
