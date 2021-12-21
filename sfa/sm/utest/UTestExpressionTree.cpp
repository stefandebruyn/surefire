#include "sfa/sm/ExpressionTree.hpp"
#include "utest/UTest.hpp"

#include <iostream>

static const ExpressionTree<I32> gA(5);
static const ExpressionTree<I32> gB(3);
static const ExpressionTree<I32> gC(-4);
static const ExpressionTree<I32> gD(10);

static const ExpressionTree<bool> gX(true);
static const ExpressionTree<bool> gY(false);
static const ExpressionTree<bool> gZ(true);

TEST_GROUP(ExpressionTree)
{
};

TEST(ExpressionTree, EvaluateConstant)
{
    I32 ans = 0;
    CHECK_SUCCESS(gA.evaluate(ans));
    CHECK_EQUAL(5, ans);
}

TEST(ExpressionTree, EvaluateElement)
{
    I32 backing = 10;
    Element<I32> elem(backing);
    const ExpressionTree<I32> node(elem);
    I32 ans = 0;
    CHECK_SUCCESS(node.evaluate(ans));
    CHECK_EQUAL(10, ans);
}

TEST(ExpressionTree, OpAdd)
{
    const ExpressionTree<I32> op(OP_ADD, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(8, ans);
}

TEST(ExpressionTree, OpSubtract)
{
    const ExpressionTree<I32> op(OP_SUBTRACT, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(2, ans);
}

TEST(ExpressionTree, OpMultiply)
{
    const ExpressionTree<I32> op(OP_MULTIPLY, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(15, ans);
}

TEST(ExpressionTree, OpDivide)
{
    const ExpressionTree<I32> op(OP_DIVIDE, &gA, &gB);
    I32 ans = 0;
    CHECK_SUCCESS(op.evaluate(ans));
    CHECK_EQUAL(1, ans);
}

TEST(ExpressionTree, CompoundArithmeticExpression)
{
    const ExpressionTree<I32> div(OP_DIVIDE, &gA, &gB);
    const ExpressionTree<I32> add(OP_ADD, &div, &gC);
    const ExpressionTree<I32> mul(OP_MULTIPLY, &add, &gD);
    I32 ans = 0;
    CHECK_SUCCESS(mul.evaluate(ans));
    CHECK_EQUAL(-30, ans);
}

TEST(ExpressionTree, OpAnd)
{
    const ExpressionTree<bool> xAndY(OP_AND, &gX, &gY);
    bool ans = true;
    CHECK_SUCCESS(xAndY.evaluate((bool&) ans));
    CHECK_EQUAL(false, ans);

    ans = true;
    const ExpressionTree<bool> yAndX(OP_AND, &gY, &gX);
    CHECK_SUCCESS(yAndX.evaluate((bool&) ans));
    CHECK_EQUAL(false, ans);

    ans = true;
    const ExpressionTree<bool> yAndY(OP_AND, &gY, &gY);
    CHECK_SUCCESS(yAndY.evaluate((bool&) ans));
    CHECK_EQUAL(false, ans);

    ans = false;
    const ExpressionTree<bool> xAndX(OP_AND, &gX, &gX);
    CHECK_SUCCESS(xAndX.evaluate((bool&) ans));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionTree, OpOr)
{
    const ExpressionTree<bool> xOrY(OP_OR, &gX, &gY);
    bool ans = false;
    CHECK_SUCCESS(xOrY.evaluate((bool&) ans));
    CHECK_EQUAL(true, ans);

    ans = false;
    const ExpressionTree<bool> yOrX(OP_OR, &gY, &gX);
    CHECK_SUCCESS(yOrX.evaluate((bool&) ans));
    CHECK_EQUAL(true, ans);

    ans = true;
    const ExpressionTree<bool> yOrY(OP_OR, &gY, &gY);
    CHECK_SUCCESS(yOrY.evaluate((bool&) ans));
    CHECK_EQUAL(false, ans);

    ans = false;
    const ExpressionTree<bool> xOrX(OP_OR, &gX, &gX);
    CHECK_SUCCESS(xOrX.evaluate((bool&) ans));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionTree, CompoundLogicalExpression)
{
    const ExpressionTree<bool> xOrY(OP_OR, &gX, &gY);
    const ExpressionTree<bool> andZ(OP_AND, &xOrY, &gZ);
    bool ans = false;
    CHECK_SUCCESS(andZ.evaluate(ans));
    CHECK_EQUAL(true, ans);
}

TEST(ExpressionTree, MixedTypeSubtrees)
{
    const ExpressionTree<F64> foo(9.81);
    const ExpressionTree<I32> bar(10);
    const ExpressionTree<I32> baz(3);
    const ExpressionTree<F32> qux(1.522f);
    const ExpressionTree<I32> div(OP_DIVIDE, &bar, &baz);
    const ExpressionTree<F32, I32, F64> add(OP_ADD, &div, &foo);
    const ExpressionTree<F32> sub(OP_SUBTRACT, &add, &qux);
    F32 ans = 0.0;
    const F32 expect = (((10 / 3) + 9.81) - 1.522f);
    CHECK_SUCCESS(sub.evaluate(ans));
    CHECK_EQUAL(expect, ans);
}
