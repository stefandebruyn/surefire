////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/core/utest/UTestExpression.cpp
/// @brief Unit tests for expression node types.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Expression.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Unit tests for expression node types.
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
