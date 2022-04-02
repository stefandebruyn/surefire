////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/utest/UTestExpression.cpp
/// @brief Unit tests for expression node types.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Expression.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Expression node tests.
///
TEST_GROUP(Expression)
{
};

///
/// @test ConstExprNode evaluates to a constant value.
///
TEST(Expression, ConstExprNode)
{
    ConstExprNode<I32> expr(10);
    CHECK_EQUAL(10, expr.evaluate());
}

///
/// @test ElementExprNode evaluates to the value of its element.
///
TEST(Expression, ElementExprNode)
{
    I32 backing = 10;
    Element<I32> elem(backing);
    ElementExprNode<I32> expr(elem);
    CHECK_EQUAL(10, expr.evaluate());
}

///
/// @test BinOpExprNode applies its operation to its operands.
///
TEST(Expression, BinOpExprNode)
{
    ConstExprNode<I32> five(5);
    ConstExprNode<I32> ten(10);
    auto mult = [] (const I32 kLhs, const I32 kRhs) -> I32
        { return (kLhs * kRhs); };
    BinOpExprNode<I32> fiveTimesTen(
        [] (I32 a, I32 b) -> I32 { return (a * b); },
        five,
        ten);
    CHECK_EQUAL(50, fiveTimesTen.evaluate());
}

///
/// @brief UnaryOpExprNode applies its operation to its operand.
///
TEST(Expression, UnaryOpExprNode)
{
    ConstExprNode<bool> t(true);
    UnaryOpExprNode<bool> notT([] (bool a) -> bool { return !a; }, t);
    CHECK_EQUAL(false, notT.evaluate());
}
