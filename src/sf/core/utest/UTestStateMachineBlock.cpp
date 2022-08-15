////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/core/utest/UTestStateMachineBlock.cpp
/// @brief Unit tests for StateMachine::Block.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/StateMachine.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Unit tests for StateMachine::Block.
///
TEST_GROUP(StateMachineBlock)
{
};

///
/// @test Executing an all-null block is a nop.
///
TEST(StateMachineBlock, BlockNull)
{
    StateMachine::Block block = {nullptr, nullptr, nullptr, nullptr, nullptr};
    CHECK_EQUAL(0, block.execute());
}

///
/// @test Executing a block executes its action.
///
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

///
/// @test Executing a block executes its next block.
///
TEST(StateMachineBlock, BlockExecuteLink)
{
    // Action `foo = foo + 1`
    I32 foo = 1;
    Element<I32> elemFoo(foo);
    ElementExprNode<I32> exprFoo(elemFoo);
    ConstExprNode<I32> expr1(1);
    BinOpExprNode<I32> fooPlus1([] (I32 a, I32 b) -> I32 { return (a + b); },
                                exprFoo,
                                expr1);
    AssignmentAction<I32> fooGetsFooPlus1(elemFoo, fooPlus1);

    // Action `foo = foo * -1`
    ConstExprNode<I32> exprNeg1(-1);
    BinOpExprNode<I32> fooTimesNeg1(
        [] (I32 a, I32 b) -> I32 { return (a * b); },
        exprFoo,
        exprNeg1);
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

///
/// @test Executing a guarded block executes its if branch if the guard is true,
/// else branch if the guard is false, and next block in either case.
///
TEST(StateMachineBlock, BlockGuard)
{
    // Expression `foo == TRUE`
    bool foo = false;
    Element<bool> elemFoo(foo);
    ElementExprNode<bool> exprFoo(elemFoo);
    ConstExprNode<bool> exprTrue(true);
    BinOpExprNode<bool> fooIsTrue(
        [] (bool a, bool b) -> bool { return (a == b); },
        exprFoo,
        exprTrue);

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
    UnaryOpExprNode<bool> notBaz([] (bool a) -> bool { return !a; },
                                 exprBaz);
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
