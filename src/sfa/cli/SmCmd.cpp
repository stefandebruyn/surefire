#include <fstream>

#include "sfa/cli/SmCmd.hpp"
#include "sfa/core/Assert.hpp"
#include "sfa/sup/StateMachineCompiler.hpp"
#include "sfa/sup/StateVectorCompiler.hpp"

I32 Cli::sm(const std::vector<std::string> kArgs)
{
    SFA_ASSERT(kArgs.size() > 0);

    if (kArgs[0] == "check")
    {
        // Check state machine config command.

        SFA_ASSERT(kArgs.size() == 3);

        // Compile state vector.
        std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
        ConfigErrorInfo err;
        Result res = StateVectorCompiler::compile(kArgs[2], svAsm, &err);

        if (res != SUCCESS)
        {
            // State vector config is invalid.
            std::cout << err.prettifyError() << std::endl;
            return EXIT_FAILURE;
        }

        StateVector sv;
        res = StateVector::create(svAsm->getConfig(), sv);
        SFA_ASSERT(res == SUCCESS);

        // Compile state machine.
        err = ConfigErrorInfo();
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
        SFA_ASSERT(false);
    }

    return EXIT_SUCCESS;
}
