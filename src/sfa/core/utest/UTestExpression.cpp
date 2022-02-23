#include "sfa/core/Expression.hpp"
#include "sfa/utest/UTest.hpp"

template<typename T, typename TLhs = T, typename TRhs = TLhs>
struct Multiply final
{
    T operator() (const TLhs kLhs, const TRhs kRhs) const
    {
        return (kLhs * kRhs);
    }
};

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
    const BinOpExpr<Multiply<I32>, I32> fiveTimesTen(five, ten);
    CHECK_EQUAL(50, fiveTimesTen.evaluate());
}
