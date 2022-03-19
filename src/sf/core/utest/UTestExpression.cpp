#include "sf/core/Expression.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(Expression)
{
};

TEST(Expression, ConstExprNode)
{
    ConstExprNode<I32> expr(10);
    CHECK_EQUAL(10, expr.evaluate());
}

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

TEST(Expression, UnaryOpExprNode)
{
    ConstExprNode<bool> t(true);
    UnaryOpExprNode<bool> notT([] (bool a) -> bool { return !a; }, t);
    CHECK_EQUAL(false, notT.evaluate());
}
