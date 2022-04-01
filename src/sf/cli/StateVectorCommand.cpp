#include <fstream>

#include "sf/cli/CliUtil.hpp"
#include "sf/cli/StateVectorCommand.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/core/Assert.hpp"
#include "sf/pal/Console.hpp"

I32 Cli::sv(const Vec<String> kArgs)
{
    // Check that arguments were passed.
    if (kArgs.size() == 0)
    {
        Cli::error() << "no arguments passed to `sv`" << std::endl;
        return EXIT_FAILURE;
    }

    if (kArgs[0] == "check")
    {
        // Validate state vector config.
        return Cli::svCheck(Vec<String>((kArgs.begin() + 1), kArgs.end()));
    }

    if (kArgs[0] == "autocode")
    {
        // Autocode state vector config.
        return Cli::svAutocode(Vec<String>((kArgs.begin() + 1), kArgs.end()));
    }

    // If we got this far, command was not recognized.
    Cli::error() << "unknown command `" << kArgs[0] << "`" << std::endl;
    return EXIT_FAILURE;
}

I32 Cli::svCheck(const Vec<String> kArgs)
{
    // Check that correct number of arguments was passed.
    if (kArgs.size() != 1)
    {
        Cli::error() << "`sv check` expects 1 argument" << std::endl;
        return EXIT_FAILURE;
    }

    // Compile state vector.
    Ref<const StateVectorAssembly> svAsm;
    ErrorInfo err;
    const Result res = StateVectorCompiler::compile(kArgs[0], svAsm, &err);

    if (res != SUCCESS)
    {
        // State vector config is invalid.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Config is valid; print some info about it.
    const Ref<const StateVectorParse> svParse = svAsm->parse();
    U32 totalElems = 0;
    U32 totalBytes = 0;

    std::cout << Console::green << "state vector config is valid\n"
              << Console::reset;

    for (const StateVectorParse::RegionParse& region : svParse->regions)
    {
        // Compute region size in bytes.
        U32 regionBytes = 0;
        totalElems += region.elems.size();
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            SF_ASSERT(elem.tokType.typeInfo != nullptr);
            regionBytes += elem.tokType.typeInfo->sizeBytes;
        }

        // Print region info.
        std::cout << "region `" << Console::cyan << region.plainName
                  << Console::reset << "`: " << Console::cyan
                  << region.elems.size() << Console::reset << " elements, "
                  << Console::cyan << regionBytes << Console::reset << " B"
                  << "\n";

        totalBytes += regionBytes;
    }

    // Print total state vector info.
    std::cout << "state vector: " << Console::cyan << totalElems
              << Console::reset << " elements, " << Console::cyan << totalBytes
              << Console::reset << " B" << std::endl;

    return EXIT_SUCCESS;
}

I32 Cli::svAutocode(const Vec<String> kArgs)
{
    // Check that correct number of arguments was passed.
    if (kArgs.size() < 3)
    {
        Cli::error() << "`sv autocode` expects at least 3 arguments"
                     << std::endl;
        return EXIT_FAILURE;
    }

    const String& svFile = kArgs[0];
    const String& autocodeFile = kArgs[1];
    const String& svName = kArgs[2];
    const Vec<String> regions((kArgs.begin() + 3), kArgs.end());

    // Tokenize state vector config.
    Vec<Token> toks;
    ErrorInfo err;
    Result res = Tokenizer::tokenize(svFile, toks, &err);
    if (res != SUCCESS)
    {
        // Tokenization failed.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Parse state vector config.
    Ref<const StateVectorParse> svParse;
    res = StateVectorParser::parse(toks, svParse, &err, regions);
    if (res != SUCCESS)
    {
        // Parsing failed.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Open autocode output file.
    std::ofstream ofs(autocodeFile, std::fstream::out);
    if (!ofs.is_open())
    {
        Cli::error() << "failed to create file `" << autocodeFile << "`"
                     << std::endl;
        return EXIT_FAILURE;
    }

    // Invoke autocoder.
    res = StateVectorAutocoder::code(ofs, svName, svParse, &err);
    if (res != SUCCESS)
    {
        // Autocoding failed.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << Console::green << "successfully generated autocode"
              << Console::reset << std::endl;

    return SUCCESS;
}
