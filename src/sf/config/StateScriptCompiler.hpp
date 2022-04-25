#ifndef SF_STATE_SCRIPT_COMPILER_HPP
#define SF_STATE_SCRIPT_COMPILER_HPP

#include <iostream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateScriptParser.hpp"

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

    Result run(ErrorInfo& kTokInfo, StateScriptAssembly::Report& kReport);

private:

    friend class StateScriptCompiler;

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

    struct Config final
    {
        U64 deltaT;
        U32 initState;
    };

    Vec<StateScriptAssembly::Section> mSections;

    const Ref<const StateMachineAssembly> mSmAsm;

    Vec<Ref<const ExpressionAssembly>> mExprAsms;

    StateScriptAssembly::Config mConfig;

    StateScriptAssembly(const Vec<StateScriptAssembly::Section>& kSections,
                        const Ref<const StateMachineAssembly> kSmAsm,
                        const Vec<Ref<const ExpressionAssembly>> kExprAsms,
                        const StateScriptAssembly::Config& kConfig);

    Result printStateVector(std::ostream& kOs);
};

class StateScriptCompiler final
{
public:

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

    StateScriptCompiler() = delete;

private:

    static Result compileOptions(const StateScriptParse::Config& kParse,
                                 const Ref<const StateMachineAssembly> kSmAsm,
                                 StateScriptAssembly::Config& kConfig,
                                 ErrorInfo* const kErr);
};

#endif
