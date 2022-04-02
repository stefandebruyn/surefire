///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef FooStateMachine_HPP
#define FooStateMachine_HPP

#include "sf/core/StateMachine.hpp"
#include "sf/core/StateVector.hpp"

namespace FooStateMachine
{

static Result getConfig(StateVector& kSv, StateMachine::Config& kSmConfig)
{
    Result res = SUCCESS;

    // State vector elements
    static Element<U32>* pElemstate = nullptr;
    res = kSv.getElement("state", pElemstate);
    if (res != SUCCESS)
    {
        return res;
    }
    static Element<U32>& elemstate = *pElemstate;

    static Element<U64>* pElemtime = nullptr;
    res = kSv.getElement("time", pElemtime);
    if (res != SUCCESS)
    {
        return res;
    }
    static Element<U64>& elemtime = *pElemtime;

    static Element<U64>* pElemn = nullptr;
    res = kSv.getElement("n", pElemn);
    if (res != SUCCESS)
    {
        return res;
    }
    static Element<U64>& elemn = *pElemn;

    static Element<U64>* pElemfib_n = nullptr;
    res = kSv.getElement("fib_n", pElemfib_n);
    if (res != SUCCESS)
    {
        return res;
    }
    static Element<U64>& elemfib_n = *pElemfib_n;

    // Local elements
    static struct
    {
        U64 i = 0ULL;
        U64 fib_im1 = 0ULL;
        U64 fib_im2 = 0ULL;
        U64 tmp = 0ULL;
        U64 T = 0;
    } localBacking;

    static Element<U64> elemi(localBacking.i);
    static Element<U64> elemfib_im1(localBacking.fib_im1);
    static Element<U64> elemfib_im2(localBacking.fib_im2);
    static Element<U64> elemtmp(localBacking.tmp);
    static Element<U64> elemT(localBacking.T);

    // State `Calculate`
    static ConstExprNode<F64> node0(1.0);
    static ElementExprNode<U64> node1(elemn);
    static UnaryOpExprNode<F64, U64> node2(safeCast<F64, U64>, node1);
    static auto node3Op = [] (F64 l, F64 r) -> F64 { return (l <= r); };
    static BinOpExprNode<F64> node3(node3Op, *&node2, *&node0);
    static UnaryOpExprNode<bool, F64> node4(safeCast<bool, F64>, node3);
    static ElementExprNode<U64> node5(elemn);
    static UnaryOpExprNode<F64, U64> node6(safeCast<F64, U64>, node5);
    static UnaryOpExprNode<U64, F64> node7(safeCast<U64, F64>, node6);
    static AssignmentAction<U64> act0(elemfib_n, *&node7);
    static TransitionAction act1(2);
    static StateMachine::Block block0{nullptr, nullptr, nullptr, &act1, nullptr};
    static StateMachine::Block block1{nullptr, nullptr, nullptr, &act0, &block0};
    static ConstExprNode<F64> node8(2.0);
    static UnaryOpExprNode<U64, F64> node9(safeCast<U64, F64>, node8);
    static AssignmentAction<U64> act2(elemi, *&node9);
    static ConstExprNode<F64> node10(1.0);
    static UnaryOpExprNode<U64, F64> node11(safeCast<U64, F64>, node10);
    static AssignmentAction<U64> act3(elemfib_im1, *&node11);
    static ConstExprNode<F64> node12(0.0);
    static UnaryOpExprNode<U64, F64> node13(safeCast<U64, F64>, node12);
    static AssignmentAction<U64> act4(elemfib_im2, *&node13);
    static StateMachine::Block block2{nullptr, nullptr, nullptr, &act4, nullptr};
    static StateMachine::Block block3{nullptr, nullptr, nullptr, &act3, &block2};
    static StateMachine::Block block4{nullptr, nullptr, nullptr, &act2, &block3};
    static StateMachine::Block block5{&node4, &block1, nullptr, nullptr, &block4};
    static ElementExprNode<U64> node14(elemn);
    static UnaryOpExprNode<F64, U64> node15(safeCast<F64, U64>, node14);
    static ElementExprNode<U64> node16(elemi);
    static UnaryOpExprNode<F64, U64> node17(safeCast<F64, U64>, node16);
    static auto node18Op = [] (F64 l, F64 r) -> F64 { return (l <= r); };
    static BinOpExprNode<F64> node18(node18Op, *&node17, *&node15);
    static UnaryOpExprNode<bool, F64> node19(safeCast<bool, F64>, node18);
    static ElementExprNode<U64> node20(elemfib_im2);
    static UnaryOpExprNode<F64, U64> node21(safeCast<F64, U64>, node20);
    static ElementExprNode<U64> node22(elemfib_im1);
    static UnaryOpExprNode<F64, U64> node23(safeCast<F64, U64>, node22);
    static auto node24Op = [] (F64 l, F64 r) -> F64 { return (l + r); };
    static BinOpExprNode<F64> node24(node24Op, *&node23, *&node21);
    static UnaryOpExprNode<U64, F64> node25(safeCast<U64, F64>, node24);
    static AssignmentAction<U64> act5(elemfib_n, *&node25);
    static ElementExprNode<U64> node26(elemfib_im1);
    static UnaryOpExprNode<F64, U64> node27(safeCast<F64, U64>, node26);
    static UnaryOpExprNode<U64, F64> node28(safeCast<U64, F64>, node27);
    static AssignmentAction<U64> act6(elemfib_im2, *&node28);
    static ElementExprNode<U64> node29(elemfib_n);
    static UnaryOpExprNode<F64, U64> node30(safeCast<F64, U64>, node29);
    static UnaryOpExprNode<U64, F64> node31(safeCast<U64, F64>, node30);
    static AssignmentAction<U64> act7(elemfib_im1, *&node31);
    static ConstExprNode<F64> node32(1.0);
    static ElementExprNode<U64> node33(elemi);
    static UnaryOpExprNode<F64, U64> node34(safeCast<F64, U64>, node33);
    static auto node35Op = [] (F64 l, F64 r) -> F64 { return (l + r); };
    static BinOpExprNode<F64> node35(node35Op, *&node34, *&node32);
    static UnaryOpExprNode<U64, F64> node36(safeCast<U64, F64>, node35);
    static AssignmentAction<U64> act8(elemi, *&node36);
    static ElementExprNode<U64> node37(elemn);
    static UnaryOpExprNode<F64, U64> node38(safeCast<F64, U64>, node37);
    static ElementExprNode<U64> node39(elemi);
    static UnaryOpExprNode<F64, U64> node40(safeCast<F64, U64>, node39);
    static auto node41Op = [] (F64 l, F64 r) -> F64 { return (l > r); };
    static BinOpExprNode<F64> node41(node41Op, *&node40, *&node38);
    static UnaryOpExprNode<bool, F64> node42(safeCast<bool, F64>, node41);
    static TransitionAction act9(2);
    static StateMachine::Block block6{nullptr, nullptr, nullptr, &act9, nullptr};
    static StateMachine::Block block7{&node42, &block6, nullptr, nullptr, nullptr};
    static StateMachine::Block block8{nullptr, nullptr, nullptr, &act8, &block7};
    static StateMachine::Block block9{nullptr, nullptr, nullptr, &act7, &block8};
    static StateMachine::Block block10{nullptr, nullptr, nullptr, &act6, &block9};
    static StateMachine::Block block11{nullptr, nullptr, nullptr, &act5, &block10};
    static StateMachine::Block block12{&node19, &block11, nullptr, nullptr, nullptr};
    static const StateMachine::StateConfig stateCalculateConfig{1, &block5, &block12, nullptr};

    // State `Done`
    static const StateMachine::StateConfig stateDoneConfig{2, nullptr, nullptr, nullptr};

    static StateMachine::StateConfig stateConfigs[] =
    {
        stateCalculateConfig,
        stateDoneConfig,
        {StateMachine::NO_STATE, nullptr, nullptr, nullptr}
    };

    static StateMachine::Config smConfig = {&elemstate, &elemT, &elemtime, stateConfigs, nullptr};
    kSmConfig = smConfig;

    return SUCCESS;
}

} // namespace FooStateMachine

#endif
