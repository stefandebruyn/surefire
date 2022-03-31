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
        I32 foo;
        F64 bar;
        bool baz;
        U64 T;
    } localBacking;

    static Element<I32> elemfoo(localBacking.foo);
    static Element<F64> elembar(localBacking.bar);
    static Element<bool> elembaz(localBacking.baz);
    static Element<U64> elemT(localBacking.T);

    // State `Foo`
    static ElementExprNode<bool> node0(elembaz);
    static UnaryOpExprNode<F64, bool> node1(safeCast<F64, bool>, node0);
    static ElementExprNode<I32> node2(elemfoo);
    static UnaryOpExprNode<F64, I32> node3(safeCast<F64, I32>, node2);
    static ConstExprNode<F64> node4(10);
    static F64 stats0ArrA[1000];
    static F64 stats0ArrB[1000];
    static ExpressionStats<F64> stats0(*&node3, stats0ArrA, stats0ArrB, 1000);
    static RollAvgNode node5(stats0);
    static ConstExprNode<F64> node6(100);
    static ElementExprNode<F64> node7(elembar);
    static UnaryOpExprNode<F64, F64> node8(safeCast<F64, F64>, node7);
    static auto node9Op = [] (F64 l, F64 r) -> F64 { return (l + r); };
    static BinOpExprNode<F64> node9(node9Op, *&node8, *&node6);
    static auto node10Op = [] (F64 l, F64 r) -> F64 { return (l < r); };
    static BinOpExprNode<F64> node10(node10Op, *&node9, *&node5);
    static auto node11Op = [] (F64 l, F64 r) -> F64 { return (l || r); };
    static BinOpExprNode<F64> node11(node11Op, *&node10, *&node1);
    static UnaryOpExprNode<bool, F64> node12(safeCast<bool, F64>, node11);
    static ConstExprNode<F64> node13(1);
    static UnaryOpExprNode<I32, F64> node14(safeCast<I32, F64>, node13);
    static AssignmentAction<I32> act0(elemfoo, *&node14);
    static const StateMachine::Block block0{nullptr, nullptr, nullptr, &act0, nullptr};
    static ConstExprNode<F64> node15(true);
    static UnaryOpExprNode<bool, F64> node16(safeCast<bool, F64>, node15);
    static AssignmentAction<bool> act1(elemqux, *&node16);
    static const StateMachine::Block block1{nullptr, nullptr, nullptr, &act1, nullptr};
    static const StateMachine::Block block2{&node12, &block0, nullptr, nullptr, &block1};
    static ConstExprNode<F64> node17(10);
    static ElementExprNode<U64> node18(elemtime);
    static UnaryOpExprNode<F64, U64> node19(safeCast<F64, U64>, node18);
    static auto node20Op = [] (F64 l, F64 r) -> F64 { return (l == r); };
    static BinOpExprNode<F64> node20(node20Op, *&node19, *&node17);
    static UnaryOpExprNode<bool, F64> node21(safeCast<bool, F64>, node20);
    static TransitionAction act2(2);
    static const StateMachine::Block block3{nullptr, nullptr, nullptr, &act2, nullptr};
    static const StateMachine::Block block4{&node21, &block3, nullptr, nullptr, nullptr};
    static const StateMachine::StateConfig stateFooConfig{1, &block2, &block4, nullptr};

    // State `Bar`
    static const StateMachine::StateConfig stateBarConfig{2, nullptr, nullptr, nullptr};

    static const StateMachine::StateConfig stateConfigs[] =
    {
        stateFooConfig,
        stateBarConfig,
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}
    }

    static const StateMachine::Config smConfig{foo, &elemT, baz, stateConfigs, nullptr};

    return StateMachine::create(smConfig, kSm);
}

} // namespace FooStateMachine

#endif
