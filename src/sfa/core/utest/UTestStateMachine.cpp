#include "sfa/core/StateMachine.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(StateMachine)
{
};

TEST(StateMachine, BlockNull)
{
    StateMachine::Block block = {nullptr, nullptr, nullptr, nullptr, nullptr};
    CHECK_EQUAL(0, block.execute());
}

TEST(StateMachine, BlockExecuteAction)
{
    // Action `foo = 10`
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    ConstExpr<I32> expr10(10);
    AssignmentAction<I32> fooGets10(elemFoo, expr10);

    // Create block containing action.
    StateMachine::Block block =
        {nullptr, nullptr, nullptr, &fooGets10, nullptr};

    // Execute block. No transition, action executes.
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(10, elemFoo.read());
}

TEST(StateMachine, BlockExecuteLink)
{
    // Expression `foo + 1`
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    ElementExpr<I32> exprFoo(elemFoo);
    ConstExpr<I32> expr1(1);
    BinOpExpr<I32> fooPlus1(add<I32>, exprFoo, expr1);

    // Action `foo = foo + 1`
    AssignmentAction<I32> fooGetsFooPlus1(elemFoo, fooPlus1);

    // ```
    // foo = foo + 1
    // foo = foo + 1
    // ```
    StateMachine::Block block1 =
        {nullptr, nullptr, nullptr, &fooGetsFooPlus1, nullptr};
    StateMachine::Block block0 =
        {nullptr, nullptr, nullptr, &fooGetsFooPlus1, &block1};

    // Execute block. No transition, actions execute.
    CHECK_EQUAL(0, block0.execute());
    CHECK_EQUAL(2, elemFoo.read());
}

TEST(StateMachine, BlockGuard)
{
    // Expression `foo == TRUE`
    bool foo = false;
    Element<bool> elemFoo(foo);
    ElementExpr<bool> exprFoo(elemFoo);
    ConstExpr<bool> exprTrue(true);
    BinOpExpr<bool> fooIsTrue(equals<bool>, exprFoo, exprTrue);

    // Action `bar = 1`
    I32 bar = 0;
    Element<I32> elemBar(bar);
    ElementExpr<I32> exprBar(elemBar);
    ConstExpr<I32> expr1(1);
    AssignmentAction<I32> barGets1(elemBar, expr1);

    // Action `bar = 2`
    ConstExpr<I32> expr2(2);
    AssignmentAction<I32> barGets2(elemBar, expr2);

    // ```
    // foo:  bar = 1
    // ELSE: bar = 2
    // ```
    StateMachine::Block ifBlock =
        {nullptr, nullptr, nullptr, &barGets1, nullptr};
    StateMachine::Block elseBlock =
        {nullptr, nullptr, nullptr, &barGets2, nullptr};
    StateMachine::Block block =
        {&fooIsTrue, &ifBlock, &elseBlock, nullptr, nullptr};

    // When `foo` is true, if branch is taken.
    elemFoo.write(true);
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(1, elemBar.read());

    // When `foo` is false, else branch is taken.
    elemFoo.write(false);
    CHECK_EQUAL(0, block.execute());
    CHECK_EQUAL(2, elemBar.read());
}