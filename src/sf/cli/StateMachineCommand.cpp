#include <fstream>

#include "sf/cli/CliUtil.hpp"
#include "sf/cli/StateMachineCommand.hpp"
#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateScriptCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/Assert.hpp"

I32 Cli::sm(const Vec<String> kArgs)
{
    // Check that arguments were passed.
    if (kArgs.size() == 0)
    {
        Cli::error() << "no arguments passed to `sm`" << std::endl;
        return EXIT_FAILURE;
    }

    Vec<String> remainingArgs((kArgs.begin() + 1), kArgs.end());
    if (kArgs[0] == "check")
    {
        // Validate state machine config.
        return Cli::smCheck(remainingArgs);
    }
    else if (kArgs[0] == "test")
    {
        // Run state script.
        return Cli::smTest(remainingArgs);
    }
    else if (kArgs[0] == "autocode")
    {
        // Generate state machine autocode.
        return Cli::smAutocode(remainingArgs);
    }

    // If we got this far, command was not recognized.
    Cli::error() << "unknown command `" << kArgs[0] << "`" << std::endl;
    return EXIT_FAILURE;
}

I32 Cli::smCheck(const Vec<String> kArgs)
{
    // Check that correct number of arguments was passed.
    if (kArgs.size() != 2)
    {
        Cli::error() << "`sm check` expects 2 arguments" << std::endl;
        return EXIT_FAILURE;
    }

    const String& svFile = kArgs[0];
    const String& smFile = kArgs[1];

    // Compile state vector.
    Ref<const StateVectorAssembly> svAsm;
    ErrorInfo err;
    Result res = StateVectorCompiler::compile(svFile, svAsm, &err);

    if (res != SUCCESS)
    {
        // State vector config is invalid.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Compile state machine.
    err = ErrorInfo();
    Ref<const StateMachineAssembly> smAsm;
    res = StateMachineCompiler::compile(smFile, svAsm, smAsm, &err);

    if (res != SUCCESS)
    {
        // State machine config is invalid.
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << Console::green << "state machine config is valid"
              << Console::reset << std::endl;

    return EXIT_SUCCESS;
}

I32 Cli::smTest(const Vec<String> kArgs)
{
    // Check that correct number of arguments was passed.
    if (kArgs.size() != 3)
    {
        Cli::error() << "`sm test` expects 3 arguments" << std::endl;
        return EXIT_FAILURE;
    }

    const String& svFile = kArgs[0];
    const String& smFile = kArgs[1];
    const String& ssFile = kArgs[2];

    // Compile state vector.
    Ref<const StateVectorAssembly> svAsm;
    ErrorInfo err;
    Result res = StateVectorCompiler::compile(svFile, svAsm, &err);
    if (res != SUCCESS)
    {
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Compile state machine.
    err = ErrorInfo();
    Ref<const StateMachineAssembly> smAsm;
    res = StateMachineCompiler::compile(smFile, svAsm, smAsm, &err);
    if (res != SUCCESS)
    {
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Compile state script.
    err = ErrorInfo();
    Ref<StateScriptAssembly> ssAsm;
    res = StateScriptCompiler::compile(ssFile, smAsm, ssAsm, &err);
    if (res != SUCCESS)
    {
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Run state script.
    StateScriptAssembly::Report report{};
    res = ssAsm->run(err, report);
    if (res != SUCCESS)
    {
        std::cout << Console::red << "error" << Console::reset
                  << ": state script run failed with internal error " << res
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Print state script report.
    std::cout << report.text << std::flush;

    // Exit with nonzero status if state script failed.
    return (report.pass ? EXIT_SUCCESS : EXIT_FAILURE);
}

I32 Cli::smAutocode(const Vec<String> kArgs)
{
    // Check that correct number of arguments was passed.
    if (kArgs.size() != 4)
    {
        Cli::error() << "`sm autocode` expects 4 arguments" << std::endl;
        return EXIT_FAILURE;
    }

    const String& svFile = kArgs[0];
    const String& smFile = kArgs[1];
    const String& autocodeFile = kArgs[2];
    const String& smName = kArgs[3];

    // Compile state vector.
    Ref<const StateVectorAssembly> svAsm;
    ErrorInfo err;
    Result res = StateVectorCompiler::compile(svFile, svAsm, &err);
    if (res != SUCCESS)
    {
        std::cout << err.prettifyError() << std::endl;
        return EXIT_FAILURE;
    }

    // Compile state machine.
    err = ErrorInfo();
    Ref<const StateMachineAssembly> smAsm;
    res = StateMachineCompiler::compile(smFile, svAsm, smAsm, &err);
    if (res != SUCCESS)
    {
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
    res = StateMachineAutocoder::code(ofs, smName, smAsm);
    if (res != SUCCESS)
    {
        Cli::error() << "autocoder failed with internal error " << res
                     << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << Console::green << "successfully generated autocode"
              << Console::reset << std::endl;

    return SUCCESS;
}
