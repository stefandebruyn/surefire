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
        Ref<const StateVectorAssembly> localSvAsm;
        Ref<Vec<StateMachine::StateConfig>> stateConfigs;
        Vec<Ref<const ExpressionAssembly>> exprAsms;
        Set<String> readOnlyElems;
        Vec<Ref<StateMachine::Block>> blocks;
        Vec<Ref<IAction>> actions;

        Workspace(const Workspace&) = delete;
        Workspace(Workspace&&) = delete;
        Workspace& operator=(const Workspace&) = delete;
        Workspace& operator=(Workspace&&) = delete;
    };

    static Result checkStateVector(const Ref<const StateMachineParse> kParse,
                                   const Ref<StateVector> kSv,
                                   StateMachineAssembly::Workspace& kWs,
                                   ErrorInfo* const kErr);

    static Result compileLocalStateVector(
        const Ref<const StateMachineParse> kParse,
        StateMachineAssembly::Workspace& kWs,
        ErrorInfo* const kErr);

    static Result initLocalElementValues(
        const Ref<const StateMachineParse> kParse,
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

    const Ref<StateMachine> mObj;
    const StateMachine::Config mConfig;
    const Ref<const StateMachineParse> mParse;
    const Ref<const StateVectorAssembly> mLocalSvAsm;
    const Vec<Ref<const ExpressionAssembly>> mExprAsms;
    const Ref<Vec<StateMachine::StateConfig>> mStateConfigs;
    const Ref<Vec<IExpressionStats*>> mExprStats;
    const Vec<Ref<StateMachine::Block>> mBlocks;
    const Vec<Ref<IAction>> mActions;

    StateMachineAssembly(
        const Ref<StateMachine> kObj,
        const StateMachine::Config kConfig,
        const Ref<const StateMachineParse> kParse,
        const Ref<const StateVectorAssembly> kLocalSvAsm,
        const Vec<Ref<const ExpressionAssembly>>& kExprAsms,
        const Ref<Vec<StateMachine::StateConfig>> kStateConfigs,
        const Ref<Vec<IExpressionStats*>> kExprStats,
        const Vec<Ref<StateMachine::Block>>& kBlocks,
        const Vec<Ref<IAction>>& kActions);
};

#endif
