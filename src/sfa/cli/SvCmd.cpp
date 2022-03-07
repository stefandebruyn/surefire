#include <fstream>

#include "sfa/cli/SvCmd.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/pal/Console.hpp"
#include "sfa/sup/ConfigUtil.hpp"
#include "sfa/sup/StateVectorCompiler.hpp"

I32 Cli::sv(const std::vector<std::string> kArgs)
{
    SFA_ASSERT(kArgs.size() > 0);

    if (kArgs[0] == "check")
    {
        // Check state vector config command.

        SFA_ASSERT(kArgs.size() == 2);

        // Compile state vector.
        std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
        ConfigErrorInfo err;
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
                auto typeInfoIt =
                    ElementTypeInfo::fromName.find(elem.tokType.str);
                SFA_ASSERT(typeInfoIt != ElementTypeInfo::fromName.end());
                const ElementTypeInfo& typeInfo = (*typeInfoIt).second;
                regionBytes += typeInfo.sizeBytes;
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
        SFA_ASSERT(false);
    }

    return EXIT_SUCCESS;
}
