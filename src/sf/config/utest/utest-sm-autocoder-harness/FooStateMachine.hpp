///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef FooStateMachine_HPP
#define FooStateMachine_HPP

#include "sf/core/StateMachine.hpp"
#include "sf/core/StateVector.hpp"

namespace FooStateMachine
{

static Result init(StateMachine& kSm, StateVector& kSv)
{
    Result res = SUCCESS;

    // State vector elements
    static Element<U64>* elemtime = nullptr;
    res = kSv.getElement("time", elemtime);
    if (res != SUCCESS)
    {
        return res;
    }

    static Element<U32>* elemstate = nullptr;
    res = kSv.getElement("state", elemstate);
    if (res != SUCCESS)
    {
        return res;
    }

    static Element<bool>* elemqux = nullptr;
    res = kSv.getElement("qux", elemqux);
    if (res != SUCCESS)
    {
        return res;
    }

    // Local elements
    static struct
    {
        F64 fib = 1.522000;
        I32 foo = 0;
        F64 bar = 10001.522000;
        bool baz = false;
        U32 windowSize = 4;
        U64 T = 0;
    } localBacking;

    static Element<F64> elemfib(localBacking.fib);
    static Element<I32> elemfoo(localBacking.foo);
    static Element<F64> elembar(localBacking.bar);
    static Element<bool> elembaz(localBacking.baz);
    static Element<U32> elemwindowSize(localBacking.windowSize);
    static Element<U64> elemT(localBacking.T);

    // State `Foo`
    static ElementExprNode<bool> node0(elembaz);
    static UnaryOpExprNode<F64, bool> node1(safeCast<F64, bool>, node0);
    static ElementExprNode<I32> node2(elemfoo);
    static UnaryOpExprNode<F64, I32> node3(safeCast<F64, I32>, node2);
    static ConstExprNode<F64> node4(2);
    static ElementExprNode<U32> node5(elemwindowSize);
    static UnaryOpExprNode<F64, U32> node6(safeCast<F64, U32>, node5);
    static auto node7Op = [] (F64 l, F64 r) -> F64 { return (l * r); };
    static BinOpExprNode<F64> node7(node7Op, *&node6, *&node4);
    static F64 stats0ArrA[8];
    static F64 stats0ArrB[8];
    static ExpressionStats<F64> stats0(*&node3, stats0ArrA, stats0ArrB, 8);
    static RollAvgNode node8(stats0);
    static ConstExprNode<F64> node9(100);
    static ElementExprNode<F64> node10(elembar);
    static UnaryOpExprNode<F64, F64> node11(safeCast<F64, F64>, node10);
    static auto node12Op = [] (F64 l, F64 r) -> F64 { return (l + r); };
    static BinOpExprNode<F64> node12(node12Op, *&node11, *&node9);
    static auto node13Op = [] (F64 l, F64 r) -> F64 { return (l < r); };
    static BinOpExprNode<F64> node13(node13Op, *&node12, *&node8);
    static auto node14Op = [] (F64 l, F64 r) -> F64 { return (l || r); };
    static BinOpExprNode<F64> node14(node14Op, *&node13, *&node1);
    static UnaryOpExprNode<bool, F64> node15(safeCast<bool, F64>, node14);
    static ConstExprNode<F64> node16(1);
    static UnaryOpExprNode<I32, F64> node17(safeCast<I32, F64>, node16);
    static AssignmentAction<I32> act0(elemfoo, *&node17);
    static const StateMachine::Block block0{nullptr, nullptr, nullptr, &act0, nullptr};
    static ConstExprNode<F64> node18(true);
    static UnaryOpExprNode<bool, F64> node19(safeCast<bool, F64>, node18);
    static AssignmentAction<bool> act1(elemqux, *&node19);
    static const StateMachine::Block block1{nullptr, nullptr, nullptr, &act1, nullptr};
    static const StateMachine::Block block2{&node15, &block0, nullptr, nullptr, &block1};
    static ConstExprNode<F64> node20(10);
    static ElementExprNode<U64> node21(elemtime);
    static UnaryOpExprNode<F64, U64> node22(safeCast<F64, U64>, node21);
    static auto node23Op = [] (F64 l, F64 r) -> F64 { return (l == r); };
    static BinOpExprNode<F64> node23(node23Op, *&node22, *&node20);
    static UnaryOpExprNode<bool, F64> node24(safeCast<bool, F64>, node23);
    static TransitionAction act2(2);
    static const StateMachine::Block block3{nullptr, nullptr, nullptr, &act2, nullptr};
    static const StateMachine::Block block4{&node24, &block3, nullptr, nullptr, nullptr};
    static const StateMachine::StateConfig stateFooConfig{1, &block2, &block4, nullptr};

    // State `Bar`
    static const StateMachine::StateConfig stateBarConfig{2, nullptr, nullptr, nullptr};

    static const StateMachine::StateConfig stateConfigs[] =
    {
        stateFooConfig,
        stateBarConfig,
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}
    }

    static IExpressionStats* exprStats[] =
    {
        &stats0,
        nullptr
    };

    static const StateMachine::Config smConfig{foo, &elemT, baz, stateConfigs, exprStats};

    return StateMachine::create(smConfig, kSm);
}

} // namespace FooStateMachine

#endif
