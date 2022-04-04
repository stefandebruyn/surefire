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
/// @file  sf/core/utest/UTestExpressionStats.cpp
/// @brief Unit tests for the ExpressionStats template.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/ExpressionStats.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Unit tests for the ExpressionStats template.
///
TEST_GROUP(ExpressionStats)
{
};

///
/// @test All stats are 0 when the rolling window is empty.
///
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

///
/// @test Expected stats are computed when the rolling window is full of the
/// same value.
///
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

///
/// @test All stats are 0 when the rolling window has size 0.
///
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

///
/// @test Stats are not updated when a null rolling window array is provided.
///
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

///
/// @test Stats are not updated when a null rolling window array is provided.
///
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

///
/// @test Stats are not updated when both provided arrays are null.
///
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

///
/// @test Rolling window mean is computed correctly.
///
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

///
/// @test Rolling window median is computed correctly.
///
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

///
/// @test Rolling window min is computed correctly.
///
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

///
/// @test Rolling window max is computed correctly.
///
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

///
/// @test Rolling window range is computed correctly.
///
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

///
/// @test Stats are not updated when both provided arrays are null.
///
TEST(ExpressionStats, NaNBecomesZero)
{
    F64 elemBacking = (0.0 / 0.0);
    Element<F64> elem(elemBacking);
    ElementExprNode<F64> expr(elem);
    F64 arrA[4];
    F64 arrB[4];
    ExpressionStats<F64> stats(expr, arrA, arrB, 4);

    // Make the rolling window look like [NaN, 1, NaN].
    stats.update();
    elem.write(1.0);
    stats.update();
    elem.write(0.0 / 0.0);
    stats.update();

    // Stats are computed as if the rolling window is [0, 1, 0].
    CHECK_EQUAL((1.0 / 3.0), stats.mean());
    CHECK_EQUAL(0.0, stats.median());
    CHECK_EQUAL(0.0, stats.min());
    CHECK_EQUAL(1.0, stats.max());
    CHECK_EQUAL(1.0, stats.range());
}
