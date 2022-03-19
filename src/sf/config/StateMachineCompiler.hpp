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

        Assembly(const StateMachine::Config kConfig,
                 const StateMachineParser::Parse& kParse,
                 const Ref<const StateVectorCompiler::Assembly> kLocalSvAsm,
                 const Vec<Ref<const ExpressionCompiler::Assembly>> kExprs,
                 StateVector* const kLocalSv);

        ~Assembly();

        const StateMachine::Config& config() const;

        const StateMachineParser::Parse& parse() const;

        StateVector& localStateVector() const;

    private:

        const StateMachine::Config mConfig;

        const StateMachineParser::Parse mParse;

        const Ref<const StateVectorCompiler::Assembly> mLocalSvAsm;

        const Vec<Ref<const ExpressionCompiler::Assembly>> mExprs;

        StateVector* const mLocalSv;
    };

    Result compile(const String kFilePath,
                   const StateVector& kSv,
                   Ref<const StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(std::istream& kIs,
                   const StateVector& kSv,
                   Ref<const StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);

    Result compile(const StateMachineParser::Parse& kParse,
                   const StateVector& kSv,
                   Ref<const StateMachineCompiler::Assembly>& kAsm,
                   ErrorInfo* const kErr);
}

#endif
