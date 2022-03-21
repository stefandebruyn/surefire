#ifndef SF_STATE_MACHINE_ASSEMBLY_HPP
#define SF_STATE_MACHINE_ASSEMBLY_HPP

#include "sf/config/ExpressionAssembly.hpp"
#include "sf/config/StateMachineParse.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/core/StateVector.hpp"

class StateMachineAssembly final
{
public:

    static Result compile(const String kFilePath,
                          const Ref<StateVector> kSv,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(std::istream& kIs,
                          const Ref<StateVector> kSv,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr);

    static Result compile(const Ref<const StateMachineParse> kParse,
                          const Ref<StateVector> kSv,
                          Ref<const StateMachineAssembly>& kAsm,
                          ErrorInfo* const kErr);

    Ref<StateMachine> get() const;

    StateMachine::Config config() const;

    Ref<const StateMachineParse> parse() const;

    Ref<StateVector> localStateVector() const;

private:

    struct Workspace final
    {
        Map<String, IElement*> elems;
        Map<String, U32> stateIds;
        Set<String> readOnlyElems;

        Ref<const StateMachineParse> smParse;
        Ref<const StateVectorAssembly> localSvAsm;
        Vec<Ref<const ExpressionAssembly>> exprAsms;
        Ref<Vec<StateMachine::StateConfig>> stateConfigs;
        Vec<Ref<StateMachine::Block>> blocks;
        Vec<Ref<IAction>> actions;
        Ref<Vec<IExpressionStats*>> exprStatArr;
        StateMachine::Config smConfig;
        Ref<StateMachine> sm;
    };

    static Result checkStateVector(const Ref<const StateMachineParse> kParse,
                                   const Ref<StateVector> kSv,
                                   StateMachineAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    static Result compileLocalStateVector(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result checkLocalElemInitExprs(
        const StateMachineParse::LocalElementParse& kInitElem,
        const Ref<const ExpressionParse> kExpr,
        const Ref<StateVector> kSv,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result initLocalElementValues(
        const Ref<const StateMachineParse> kParse,
        const Ref<StateVector> kSv,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result compileAction(
        const Ref<const StateMachineParse::ActionParse> kParse,
        const Ref<StateVector> kSv,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<IAction>& kAction,
        ErrorInfo* const kErr);

    static Result compileBlock(
        const Ref<const StateMachineParse::BlockParse> kParse,
        const Ref<StateVector> kSv,
        StateMachineAssembly::Workspace& kWs,
        const bool kInExitLabel,
        Ref<StateMachine::Block>& kBlock,
        ErrorInfo* const kErr);

    static Result compileState(const StateMachineParse::StateParse& kParse,
                               Ref<StateVector> kSv,
                               StateMachineAssembly::Workspace& kWs,
                               ErrorInfo* const kErr);

    const StateMachineAssembly::Workspace mWs;

    StateMachineAssembly(const StateMachineAssembly::Workspace& kWs);
};

#endif
