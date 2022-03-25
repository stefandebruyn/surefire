#ifndef SF_STATE_SCRIPT_ASSEMBLY_HPP
#define SF_STATE_SCRIPT_ASSEMBLY_HPP

#include <iostream>

#include "sf/config/StateMachineAssembly.hpp"
#include "sf/config/StateScriptParse.hpp"

class StateScriptAssembly final
{
public:

    struct Report final
    {
        bool pass;
        U64 steps;
        U64 asserts;
        String text;
    };

    static Result compile(const String kFilePath,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(std::istream& kIs,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(const Ref<const StateScriptParse> kParse,
                          const Ref<const StateMachineAssembly> kSmAsm,
                          Ref<StateScriptAssembly>& kAsm,
                          ErrorInfo* const kErr);

    Result run(ErrorInfo& kTokInfo, StateScriptAssembly::Report& kReport);

private:

    struct Input final
    {
        IExprNode<bool>* guard;
        Ref<IAction> action;
    };

    struct Assert final
    {
        IExprNode<bool>* guard;
        IExprNode<bool>* assert;
        Token tokAssert;
    };

    struct Section final
    {
        U32 stateId;
        Vec<StateScriptAssembly::Input> inputs;
        Vec<StateScriptAssembly::Assert> asserts;
    };

    Vec<StateScriptAssembly::Section> mSections;

    const Ref<const StateMachineAssembly> mSmAsm;

    Vec<Ref<const ExpressionAssembly>> mExprAsms;

    StateScriptParse::Config mConfig;

    StateScriptAssembly(const Vec<StateScriptAssembly::Section>& kSections,
                        const Ref<const StateMachineAssembly> kSmAsm,
                        const Vec<Ref<const ExpressionAssembly>> kExprAsms,
                        const StateScriptParse::Config& kConfig);

    Result printStateVector(std::ostream& kOs);
};

#endif
