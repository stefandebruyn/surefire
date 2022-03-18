#ifndef SF_STATE_MACHINE_COMPILER_HPP
#define SF_STATE_MACHINE_COMPILER_HPP

#include "sf/config/ExpressionCompiler.hpp"
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
            const std::shared_ptr<StateVectorCompiler::Assembly> kLocalSvAsm,
            const std::vector<std::shared_ptr<ExpressionCompiler::Assembly>>
                kExprs,
            StateVector* const kLocalSv);

        ~Assembly();

        const StateMachine::Config& config() const;

        const StateMachineParser::Parse& parse() const;

        StateVector& localStateVector() const;

    private:

        const StateMachine::Config mConfig;

        const StateMachineParser::Parse mParse;

        const std::shared_ptr<StateVectorCompiler::Assembly> mLocalSvAsm;

        const std::vector<std::shared_ptr<ExpressionCompiler::Assembly>> mExprs;

        StateVector* const mLocalSv;
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
