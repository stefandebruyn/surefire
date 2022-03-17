#include "sf/core/StateMachine.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(StateMachineBlock)
{
};

TEST(StateMachineBlock, BlockNull)
{
    StateMachine::Block block = {nullptr, nullptr, nullptr, nullptr, nullptr};
    CHECK_EQUAL(0, block.execute());
}

TEST(StateMachineBlock, BlockExecuteAction)
{
    // Action `foo = 10`
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    ConstExprNode<I32> expr10(10);
    AssignmentAction<I32> fooGets10(elemFoo, expr10);

    // Create block containing action.
    StateMachine::Block block =
        {nullptr, nullptr, nullptr, &fooGets10, nullptr};

    // Execute block. No transition, action executes.
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(10, elemFoo.read());
}

TEST(StateMachineBlock, BlockExecuteLink)
{
    // Action `foo = foo + 1`
    I32 foo = 1;
    Element<I32> elemFoo(foo);
    ElementExprNode<I32> exprFoo(elemFoo);
    ConstExprNode<I32> expr1(1);
    BinOpExprNode<I32> fooPlus1(add<I32>, exprFoo, expr1);
    AssignmentAction<I32> fooGetsFooPlus1(elemFoo, fooPlus1);

    // Action `foo = foo * -1`
    ConstExprNode<I32> exprNeg1(-1);
    BinOpExprNode<I32> fooTimesNeg1(multiply<I32>, exprFoo, exprNeg1);
    AssignmentAction<I32> fooGetsFooTimesNeg1(elemFoo, fooTimesNeg1);

    // foo = foo + 1
    // foo = foo * -1
    StateMachine::Block block1 =
        {nullptr, nullptr, nullptr, &fooGetsFooTimesNeg1, nullptr};
    StateMachine::Block block0 =
        {nullptr, nullptr, nullptr, &fooGetsFooPlus1, &block1};

    // Execute block. No transition, actions execute in the expected order.
    CHECK_EQUAL(0, block0.execute());
    CHECK_EQUAL(-2, elemFoo.read());
}

TEST(StateMachineBlock, BlockGuard)
{
    // Expression `foo == TRUE`
    bool foo = false;
    Element<bool> elemFoo(foo);
    ElementExprNode<bool> exprFoo(elemFoo);
    ConstExprNode<bool> exprTrue(true);
    BinOpExprNode<bool> fooIsTrue(equals<bool>, exprFoo, exprTrue);

    // Action `bar = 1`
    I32 bar = 0;
    Element<I32> elemBar(bar);
    ElementExprNode<I32> exprBar(elemBar);
    ConstExprNode<I32> expr1(1);
    AssignmentAction<I32> barGets1(elemBar, expr1);

    // Action `bar = 2`
    ConstExprNode<I32> expr2(2);
    AssignmentAction<I32> barGets2(elemBar, expr2);

    // Action `baz = NOT baz`
    bool baz = false;
    Element<bool> elemBaz(baz);
    ElementExprNode<bool> exprBaz(elemBaz);
    UnaryOpExprNode<bool> notBaz(bang<bool>, exprBaz);
    AssignmentAction<bool> bazGetsNotBaz(elemBaz, notBaz);

    // foo:  bar = 1
    // ELSE: bar = 2
    // baz = NOT baz
    StateMachine::Block next = {nullptr, nullptr, nullptr, &bazGetsNotBaz};
    StateMachine::Block ifBlock =
        {nullptr, nullptr, nullptr, &barGets1, nullptr};
    StateMachine::Block elseBlock =
        {nullptr, nullptr, nullptr, &barGets2, nullptr};
    StateMachine::Block block =
        {&fooIsTrue, &ifBlock, &elseBlock, nullptr, &next};

    // When `foo` is true, if branch is taken.
    elemFoo.write(true);
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(1, elemBar.read());

    // Linked block executes regardless of guard.
    CHECK_EQUAL(true, elemBaz.read());

    // When `foo` is false, else branch is taken.
    elemFoo.write(false);
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(2, elemBar.read());

    // Linked block executes regardless of guard.
    CHECK_EQUAL(false, elemBaz.read());
}
