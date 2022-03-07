#ifndef SFA_STATE_MACHINE_COMPILER_HPP
#define SFA_STATE_MACHINE_COMPILER_HPP

#include "sfa/core/StateVector.hpp"
#include "sfa/sup/StateMachineParser.hpp"

namespace StateMachineCompiler
{
    class Assembly final
    {
    public:

        Assembly(const StateMachine::Config kConfig,
                 const StateMachineParser::Parse& kParse);

        ~Assembly();

        const StateMachine::Config& getConfig() const;

        const StateMachineParser::Parse& getParse() const;

    private:

        const StateMachine::Config mConfig;

        const StateMachineParser::Parse mParse;
    };

    Result compile(const std::string kFilePath,
                   const StateVector& kSv,
                   std::shared_ptr<Assembly>& kAsm,
                   ConfigErrorInfo* const kConfigErr);

    Result compile(std::istream& kIs,
                   const StateVector& kSv,
                   std::shared_ptr<Assembly>& kAsm,
                   ConfigErrorInfo* const kConfigErr);

    Result compile(const StateMachineParser::Parse& kParse,
                   const StateVector& kSv,
                   std::shared_ptr<Assembly>& kAsm,
                   ConfigErrorInfo* const kConfigErr);
}

#endif
