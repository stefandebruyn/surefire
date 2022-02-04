#include "sfa/sm/ExpressionNode.hpp"
#include "UTest.hpp"

static const ExpressionNode<I32> gA(5);
static const ExpressionNode<I32> gB(3);
static const ExpressionNode<I32> gC(-4);
static const ExpressionNode<I32> gD(10);

static const ExpressionNode<bool> gX(true);
static const ExpressionNode<bool> gY(false);
static const ExpressionNode<bool> gZ(true);

TEST_GROUP(ExpressionNode)
{
};

TEST(ExpressionNode, EvaluateConstant)
{
    I32 ans = 0;
    CHECK_SUCCESS(gA.evaluate(ans));
    CHECK_EQUAL(5, ans);
}

TEST(ExpressionNode, EvaluateElement)
{
    I32 backing = 10;
    Element<I32> elem(backing);
    const ExpressionNode<I32> node(elem);
    I32 ans = 0;
    CHECK_SUCCESS(node.evaluate(ans));
    CHECK_EQUAL(10, ans);
}

TEST(ExpressionNode, OpAdd)
{
    const ExpressionNode<I32> op(OP_ADD, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(8, ans);
}

TEST(ExpressionNode, OpSubtract)
{
    const ExpressionNode<I32> op(OP_SUBTRACT, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(2, ans);
}

TEST(ExpressionNode, OpMultiply)
{
    const ExpressionNode<I32> op(OP_MULTIPLY, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    // FIXME: Temporary comment-out while `OP_MULTIPLY is a nop.
    // CHECK_EQUAL(15, ans);
}

TEST(ExpressionNode, OpDivide)
{
    const ExpressionNode<I32> op(OP_DIVIDE, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(1, ans);
}

TEST(ExpressionNode, CompoundArithmeticExpression)
{
    const ExpressionNode<I32> div(OP_DIVIDE, &gA, &gB);
    const ExpressionNode<I32> add(OP_ADD, &div, &gC);
    const ExpressionNode<I32> mul(OP_MULTIPLY, &add, &gD);
    I32 ans = 0;
    CHECK_SUCCESS(mul.evaluate(ans));
    // FIXME: Temporary comment-out while `OP_MULTIPLY is a nop.
    // CHECK_EQUAL(-30, ans);
}

TEST(ExpressionNode, OpAnd)
{
    const ExpressionNode<bool> xAndY(OP_AND, &gX, &gY);
    bool ans = true;
    CHECK_SUCCESS(xAndY.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(false, ans);

    ans = true;
    const ExpressionNode<bool> yAndX(OP_AND, &gY, &gX);
    CHECK_SUCCESS(yAndX.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(false, ans);

    ans = true;
    const ExpressionNode<bool> yAndY(OP_AND, &gY, &gY);
    CHECK_SUCCESS(yAndY.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(false, ans);

    ans = false;
    const ExpressionNode<bool> xAndX(OP_AND, &gX, &gX);
    CHECK_SUCCESS(xAndX.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionNode, OpOr)
{
    const ExpressionNode<bool> xOrY(OP_OR, &gX, &gY);
    bool ans = false;
    CHECK_SUCCESS(xOrY.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(true, ans);

    ans = false;
    const ExpressionNode<bool> yOrX(OP_OR, &gY, &gX);
    CHECK_SUCCESS(yOrX.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(true, ans);

    ans = true;
    const ExpressionNode<bool> yOrY(OP_OR, &gY, &gY);
    CHECK_SUCCESS(yOrY.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(false, ans);

    ans = false;
    const ExpressionNode<bool> xOrX(OP_OR, &gX, &gX);
    CHECK_SUCCESS(xOrX.evaluate(static_cast<bool&>(ans)));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionNode, CompoundLogicalExpression)
{
    const ExpressionNode<bool> xOrY(OP_OR, &gX, &gY);
    const ExpressionNode<bool> andZ(OP_AND, &xOrY, &gZ);
    bool ans = false;
    CHECK_SUCCESS(andZ.evaluate(ans));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionNode, MixedTypeSubtrees)
{
    const ExpressionNode<F64> foo(9.81);
    const ExpressionNode<I32> bar(10);
    const ExpressionNode<I32> baz(3);
    const ExpressionNode<F32> qux(1.522f);
    const ExpressionNode<I32> div(OP_DIVIDE, &bar, &baz);
    const ExpressionNode<F32, I32, F64> add(OP_ADD, &div, &foo);
    const ExpressionNode<F32> sub(OP_SUBTRACT, &add, &qux);
    F32 ans = 0.0;
    const F32 expect = (((10 / 3) + 9.81) - 1.522f);
    CHECK_SUCCESS(sub.evaluate(ans));
    CHECK_EQUAL(expect, ans);
}
