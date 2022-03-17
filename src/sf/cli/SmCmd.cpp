#include <fstream>

#include "sf/cli/SmCmd.hpp"
#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/Assert.hpp"

I32 Cli::sm(const std::vector<std::string> kArgs)
{
    SF_ASSERT(kArgs.size() > 0);

    if (kArgs[0] == "check")
    {
        // Check state machine config command.

        SF_ASSERT(kArgs.size() == 3);

        // Compile state vector.
        std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
        ErrorInfo err;
        Result res = StateVectorCompiler::compile(kArgs[2], svAsm, &err);

        if (res != SUCCESS)
        {
            // State vector config is invalid.
            std::cout << err.prettifyError() << std::endl;
            return EXIT_FAILURE;
        }

        StateVector sv;
        res = StateVector::create(svAsm->getConfig(), sv);
        SF_ASSERT(res == SUCCESS);

        // Compile state machine.
        err = ErrorInfo();
        std::shared_ptr<StateMachineCompiler::Assembly> smAsm;
        res = StateMachineCompiler::compile(kArgs[1], sv, smAsm, &err);

        if (res != SUCCESS)
        {
            // State machine config is invalid.
            std::cout << err.prettifyError() << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << Console::green << "state machine config is valid"
                    << Console::reset << std::endl;
    }
    else
    {
        SF_ASSERT(false);
    }

    return EXIT_SUCCESS;
}
