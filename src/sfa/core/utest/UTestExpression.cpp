#include "sfa/core/Expression.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(Expression)
{
};

TEST(Expression, ConstExprNode)
{
    const ConstExprNode<I32> expr(10);
    CHECK_EQUAL(10, expr.evaluate());
}

TEST(Expression, BinOpExprNode)
{
    const ConstExprNode<I32> five(5);
    const ConstExprNode<I32> ten(10);
    const BinOpExprNode<I32> fiveTimesTen(multiply<I32>, five, ten);
    CHECK_EQUAL(50, fiveTimesTen.evaluate());
}

TEST(Expression, UnaryOpExprNode)
{
    const ConstExprNode<bool> t(true);
    UnaryOpExprNode<bool> notT(bang<bool>, t);
    CHECK_EQUAL(false, notT.evaluate());
}
