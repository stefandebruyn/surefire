#include "sf/core/ExpressionStats.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(ExpressionStats)
{
};

TEST(ExpressionStats, EmptyHistory)
{
    ConstExprNode<I32> expr(0);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);
    CHECK_EQUAL(0.0, stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(0.0, stats.min());
    CHECK_EQUAL(0.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, AllSameValue)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(10);
    stats.update();
    elem.write(10);
    stats.update();
    elem.write(10);
    stats.update();
    elem.write(10);
    stats.update();

    CHECK_EQUAL(10.0, stats.mean());
    CHECK_EQUAL(10.0, stats.median());
    CHECK_EQUAL(10.0, stats.min());
    CHECK_EQUAL(10.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, ZeroSize)
{
    ConstExprNode<I32> expr(0);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 0);

    stats.update();
    stats.update();
    stats.update();

    CHECK_EQUAL(0.0, stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(0.0, stats.min());
    CHECK_EQUAL(0.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, NullHistoryArray)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, nullptr, arrB, 4);

    elem.write(10);
    stats.update();
    CHECK_EQUAL(0.0, stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(0.0, stats.min());
    CHECK_EQUAL(0.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, NullSortedArray)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    ExpressionStats<I32> stats(expr, arrA, nullptr, 4);

    elem.write(10);
    stats.update();
    CHECK_EQUAL(10.0, stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(10.0, stats.min());
    CHECK_EQUAL(10.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, BothArraysNull)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    ExpressionStats<I32> stats(expr, nullptr, nullptr, 4);

    elem.write(10);
    stats.update();
    CHECK_EQUAL(0.0, stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(0.0, stats.min());
    CHECK_EQUAL(0.0, stats.max());
    CHECK_EQUAL(0.0, stats.range());
}

TEST(ExpressionStats, Mean)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(1);
    stats.update();
    CHECK_EQUAL(1.0, stats.mean());

    elem.write(2);
    stats.update();
    CHECK_EQUAL(((1.0 + 2.0) / (2.0)), stats.mean());

    elem.write(7);
    stats.update();
    CHECK_EQUAL(((1.0 + 2.0 + 7.0) / (3.0)), stats.mean());

    elem.write(-40);
    stats.update();
    CHECK_EQUAL(((1.0 + 2.0 + 7.0 + -40.0) / (4.0)), stats.mean());

    elem.write(11);
    stats.update();
    CHECK_EQUAL(((2.0 + 7.0 + -40.0 + 11.0) / (4.0)), stats.mean());
}

TEST(ExpressionStats, Median)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(1);
    stats.update();
    CHECK_EQUAL(1.0, stats.median());

    elem.write(2);
    stats.update();
    CHECK_EQUAL(((1.0 + 2.0) / (2.0)), stats.median());

    elem.write(-10);
    stats.update();
    CHECK_EQUAL(1.0, stats.median());

    elem.write(-40);
    stats.update();
    CHECK_EQUAL(((-10 + 1.0) / (2.0)), stats.median());

    elem.write(100);
    stats.update();
    CHECK_EQUAL(((-10.0 + 2.0) / (2.0)), stats.median());
}

TEST(ExpressionStats, Min)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(1);
    stats.update();
    CHECK_EQUAL(1.0, stats.min());

    elem.write(2);
    stats.update();
    CHECK_EQUAL(1.0, stats.min());

    elem.write(7);
    stats.update();
    CHECK_EQUAL(1.0, stats.min());

    elem.write(-40);
    stats.update();
    CHECK_EQUAL(-40.0, stats.min());

    elem.write(11);
    stats.update();
    CHECK_EQUAL(-40.0, stats.min());

    elem.write(0);
    stats.update();
    CHECK_EQUAL(-40.0, stats.min());

    elem.write(-39);
    stats.update();
    CHECK_EQUAL(-40.0, stats.min());

    elem.write(20);
    stats.update();
    CHECK_EQUAL(-39.0, stats.min());
}

TEST(ExpressionStats, Max)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(1);
    stats.update();
    CHECK_EQUAL(1.0, stats.max());

    elem.write(2);
    stats.update();
    CHECK_EQUAL(2.0, stats.max());

    elem.write(0);
    stats.update();
    CHECK_EQUAL(2.0, stats.max());

    elem.write(40);
    stats.update();
    CHECK_EQUAL(40.0, stats.max());

    elem.write(11);
    stats.update();
    CHECK_EQUAL(40.0, stats.max());

    elem.write(0);
    stats.update();
    CHECK_EQUAL(40.0, stats.max());

    elem.write(39);
    stats.update();
    CHECK_EQUAL(40.0, stats.max());

    elem.write(20);
    stats.update();
    CHECK_EQUAL(39.0, stats.max());
}

TEST(ExpressionStats, Range)
{
    I32 elemBacking = 0;
    Element<I32> elem(elemBacking);
    ElementExprNode<I32> expr(elem);
    I32 arrA[4];
    I32 arrB[4];
    ExpressionStats<I32> stats(expr, arrA, arrB, 4);

    elem.write(1);
    stats.update();
    CHECK_EQUAL(0.0, stats.range());

    elem.write(2);
    stats.update();
    CHECK_EQUAL(1.0, stats.range());

    elem.write(7);
    stats.update();
    CHECK_EQUAL(6.0, stats.range());

    elem.write(-40);
    stats.update();
    CHECK_EQUAL(47.0, stats.range());

    elem.write(11);
    stats.update();
    CHECK_EQUAL(51.0, stats.range());

    elem.write(0);
    stats.update();
    CHECK_EQUAL(51.0, stats.range());

    elem.write(-39);
    stats.update();
    CHECK_EQUAL(51.0, stats.range());

    elem.write(5);
    stats.update();
    CHECK_EQUAL(50.0, stats.range());
}
