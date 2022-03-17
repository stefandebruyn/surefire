#ifndef SF_STATE_MACHINE_COMPILER_HPP
#define SF_STATE_MACHINE_COMPILER_HPP

#include "sf/config/StateMachineParser.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/StateVector.hpp"

namespace StateMachineCompiler
{
    class Assembly final
    {
    public:

        Assembly(
            const StateMachine::Config kConfig,
            const StateMachineParser::Parse& kParse,
            const std::shared_ptr<StateVectorCompiler::Assembly> kLocalSvASm);

        ~Assembly();

        const StateMachine::Config& getConfig() const;

        const StateMachineParser::Parse& getParse() const;

    private:

        const StateMachine::Config mConfig;

        const StateMachineParser::Parse mParse;

        const std::shared_ptr<StateVectorCompiler::Assembly> mLocalSvAsm;
    };

    Result compile(const std::string kFilePath,
                   const StateVector& kSv,
                   std::shared_ptr<StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(std::istream& kIs,
                   const StateVector& kSv,
                   std::shared_ptr<StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(const StateMachineParser::Parse& kParse,
                   const StateVector& kSv,
                   std::shared_ptr<StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif
