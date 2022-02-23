#include "sfa/core/Expression.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(Expression)
{
};

TEST(Expression, ConstExpr)
{
    const ConstExpr<I32> expr(10);
    CHECK_EQUAL(10, expr.evaluate());
}

TEST(Expression, BinOpExpr)
{
    const ConstExpr<I32> five(5);
    const ConstExpr<I32> ten(10);
    const BinOpExpr<I32> fiveTimesTen(multiply<I32>, five, ten);
    CHECK_EQUAL(50, fiveTimesTen.evaluate());
}

TEST(Expression, UnaryOpExpr)
{
    const ConstExpr<bool> t(true);
    UnaryOpExpr<bool> notT(bang<bool>, t);
    CHECK_EQUAL(false, notT.evaluate());
}
