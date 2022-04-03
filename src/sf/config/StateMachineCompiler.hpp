#ifndef SF_STATE_MACHINE_COMPILER_HPP
#define SF_STATE_MACHINE_COMPILER_HPP

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/StateMachineParser.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/core/StateVector.hpp"

class StateMachineAssembly final
{
public:

    StateMachine& get() const;

    StateMachine::Config config() const;

    StateVector& localStateVector() const;

    Ref<const StateMachineParse> parse() const;

    struct Workspace final
    {
        Map<String, IElement*> elems;
        Map<String, U32> stateIds;
        Set<String> readOnlyElems;

        Ref<const StateVectorAssembly> svAsm;
        Ref<const StateVectorAssembly> localSvAsm;
        Vec<Ref<const ExpressionAssembly>> exprAsms;
        Ref<Vec<StateMachine::StateConfig>> stateConfigs;
        Vec<Ref<StateMachine::Block>> blocks;
        Vec<Ref<IAction>> actions;
        Ref<Vec<IExpressionStats*>> exprStatArr;

        Ref<StateMachine> sm;
        StateMachine::Config smConfig;
        Ref<const StateMachineParse> smParse;
    };

    const StateMachineAssembly::Workspace mWs;

private:

    friend class StateMachineCompiler;

    friend class StateMachineAutocoder;

    friend class StateScriptCompiler;

    friend class StateScriptAssembly;

    StateMachineAssembly(const StateMachineAssembly::Workspace& kWs);
};

class StateMachineCompiler final
{
public:

    static const String FIRST_STATE;

    static Result compile(const String kFilePath,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE);

    static Result compile(std::istream& kIs,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE);

    static Result compile(const Ref<const StateMachineParse> kParse,
                          const Ref<const StateVectorAssembly> kSvAsm,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr,
                          const String kInitState = FIRST_STATE);

    StateMachineCompiler() = delete;

private:

    friend class StateScriptCompiler;

    static bool stateNameReserved(const Token& kTokSection);

    static Result checkStateVector(const Ref<const StateMachineParse> kParse,
                                   StateMachineAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    static Result compileLocalStateVector(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result checkLocalElemInitExprs(
        const StateMachineParse::LocalElementParse& kInitElem,
        const Ref<const ExpressionParse> kExpr,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result initLocalElementValues(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result compileAssignmentAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        const Map<String, IElement*>& kBindings,
        const Set<String>& kReadOnlyElems,
        Ref<IAction>& kAction,
        Ref<const ExpressionAssembly>& kRhsAsm,
        ErrorInfo* const kErr);

    static Result compileAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<IAction>& kAction,
        ErrorInfo* const kErr);

    static Result compileBlock(
        const Ref<const StateMachineParse::BlockParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<StateMachine::Block>& kBlock,
        ErrorInfo* const kErr);

    static Result compileState(const StateMachineParse::StateParse& kParse,
                               StateMachineAssembly::Workspace& kWs,
                               ErrorInfo* const kErr);
};

#endif
