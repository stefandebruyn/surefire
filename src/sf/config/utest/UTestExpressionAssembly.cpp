#include <cstring>

#include "sf/config/ExpressionAssembly.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define PARSE_EXPR(kExprSrc)                                                   \
    TOKENIZE(kExprSrc);                                                        \
    Ref<const ExpressionParse> exprParse;                                      \
    CHECK_SUCCESS(ExpressionParse::parse(it, exprParse, nullptr));

static void checkEvalConstExpr(const char* const kExprSrc, const F64 kExpectVal)
{
    PARSE_EXPR(kExprSrc);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>*>(exprAsm->root().get());
    CHECK_EQUAL(kExpectVal, root->evaluate());
}

static void checkCompileError(const Ref<const ExpressionParse> kExprParse,
                              const Map<String, IElement*> kBindings,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Got expected return code from compiler.
    Ref<const ExpressionAssembly> exprAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, ExpressionAssembly::compile(kExprParse,
                                                  kBindings,
                                                  ElementType::FLOAT64,
                                                  exprAsm,
                                                  &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Assembly pointer was not populated.
    CHECK_TRUE(exprAsm == nullptr);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, ExpressionAssembly::compile(kExprParse,
                                                  kBindings,
                                                  ElementType::FLOAT64,
                                                  exprAsm,
                                                  nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(ExpressionAssembly)
{
};

TEST(ExpressionAssembly, SimplePrecedence)
{
    checkEvalConstExpr("1 + 2 * 3", 7);
}

TEST(ExpressionAssembly, SimplePrecedenceWithParens)
{
    checkEvalConstExpr("(1 + 2) * 3", 9);
}

TEST(ExpressionAssembly, Not)
{
    checkEvalConstExpr("NOT FALSE", 1.0);
    checkEvalConstExpr("NOT TRUE", 0.0);
    checkEvalConstExpr("NOT NOT FALSE", 0.0);
    checkEvalConstExpr("NOT NOT NOT FALSE", 1.0);
}

TEST(ExpressionAssembly, Multiply)
{
    checkEvalConstExpr("5 * 3", (5 * 3));
    checkEvalConstExpr("5 * 3 * -3.14 * 9.81 * -1.62",
                       (5 * 3 * -3.14 * 9.81 * -1.62));
    checkEvalConstExpr("5 * (3 * (-3.14 * 9.81)) * -1.62",
                       (5 * (3 * (-3.14 * 9.81)) * -1.62));
}

TEST(ExpressionAssembly, Divide)
{
    checkEvalConstExpr("5 / 3", (5.0 / 3.0));
    checkEvalConstExpr("5 / 3 / -3.14 / 9.81 / -1.62",
                       (5.0 / 3 / -3.14 / 9.81 / -1.62));
    checkEvalConstExpr("5 / (3 / (-3.14 / 9.81)) / -1.62",
                       (5.0 / (3 / (-3.14 / 9.81)) / -1.62));
}

TEST(ExpressionAssembly, Add)
{
    checkEvalConstExpr("5 + 3", (5.0 + 3.0));
    checkEvalConstExpr("5 + 3 + -3.14 + 9.81 + -1.62",
                       (5 + 3 + -3.14 + 9.81 + -1.62));
    checkEvalConstExpr("5 + (3 + (-3.14 + 9.81)) + -1.62",
                       (5 + (3 + (-3.14 + 9.81)) + -1.62));
}

TEST(ExpressionAssembly, Subtract)
{
    checkEvalConstExpr("5 - 3", (5.0 - 3.0));
    checkEvalConstExpr("5 - 3 - -3.14 - 9.81 - -1.62",
                       (5 - 3 - -3.14 - 9.81 - -1.62));
    checkEvalConstExpr("5 - (3 - (-3.14 - 9.81)) - -1.62",
                       (5 - (3 - (-3.14 - 9.81)) - -1.62));
}

TEST(ExpressionAssembly, ComplexArithmetic)
{
    const F64 expectVal =
        (4789.478932478923 * (-321.5789004 - 333.47823 * 0.07849327843)
         / 3789.047893274982 * (10.743808 + (-1.0 / 3.0) - 900.9009) + -123456
         * ((-405.78 + 500.4333 + 7 + (7.7 + 7.77)) + 7.777 * 10.9 / 2.555));
    checkEvalConstExpr(
        "(4789.478932478923 * (-321.5789004 - 333.47823 * 0.07849327843)"
        " / 3789.047893274982 * (10.743808 + (-1.0 / 3.0) - 900.9009) + -123456"
        " * ((-405.78 + 500.4333 + 7 + (7.7 + 7.77)) + 7.777 * 10.9 / 2.555))",
        expectVal);
}

TEST(ExpressionAssembly, LessThan)
{
    checkEvalConstExpr("3 < 5", 1.0);
    checkEvalConstExpr("5 < 3", 0.0);
    checkEvalConstExpr("5 < 5", 0.0);
}

TEST(ExpressionAssembly, LessThanEqual)
{
    checkEvalConstExpr("3 <= 5", 1.0);
    checkEvalConstExpr("5 <= 3", 0.0);
    checkEvalConstExpr("5 <= 5", 1.0);
}

TEST(ExpressionAssembly, GreaterThan)
{
    checkEvalConstExpr("5 > 3", 1.0);
    checkEvalConstExpr("3 > 5", 0.0);
    checkEvalConstExpr("5 > 5", 0.0);
}

TEST(ExpressionAssembly, GreaterThanEqual)
{
    checkEvalConstExpr("5 >= 3", 1.0);
    checkEvalConstExpr("3 >= 5", 0.0);
    checkEvalConstExpr("5 >= 5", 1.0);
}

TEST(ExpressionAssembly, Equal)
{
    checkEvalConstExpr("5 == 5", 1.0);
    checkEvalConstExpr("3 == 5", 0.0);
}

TEST(ExpressionAssembly, NotEqual)
{
    checkEvalConstExpr("3 != 5", 1.0);
    checkEvalConstExpr("5 != 5", 0.0);
}

TEST(ExpressionAssembly, And)
{
    checkEvalConstExpr("FALSE AND FALSE", 0.0);
    checkEvalConstExpr("FALSE AND TRUE", 0.0);
    checkEvalConstExpr("TRUE AND FALSE", 0.0);
    checkEvalConstExpr("TRUE AND TRUE", 1.0);
}

TEST(ExpressionAssembly, Or)
{
    checkEvalConstExpr("FALSE OR FALSE", 0.0);
    checkEvalConstExpr("FALSE OR TRUE", 1.0);
    checkEvalConstExpr("TRUE OR FALSE", 1.0);
    checkEvalConstExpr("TRUE OR TRUE", 1.0);
}

TEST(ExpressionAssembly, ComplexLogic)
{
    const bool expectVal =
        (true || !(false && true && !(false && !false)) || true && false
         || ((true && !true || false) || !!true) || false && true || false
         && (!false && true) && !(!(true || false) || (!false || true)));
    checkEvalConstExpr(
        "(TRUE OR !(FALSE AND TRUE AND !(FALSE AND !FALSE)) OR TRUE AND FALSE"
        " OR ((TRUE AND !TRUE OR FALSE) OR !!TRUE) OR FALSE AND TRUE OR FALSE"
        " AND (!FALSE AND TRUE) AND !(!(TRUE OR FALSE) OR (!FALSE OR TRUE)))",
        expectVal);
}

TEST(ExpressionAssembly, MixedArithmeticAndLogic)
{
    checkEvalConstExpr("(4 + 6) / 2 == (100 - 120) / (4 * -1)", 1.0);
}

TEST(ExpressionAssembly, DoubleInequalityLt)
{
    checkEvalConstExpr("1 < 2 < 3", 1.0);
    checkEvalConstExpr("2 < 2 < 3", 0.0);
    checkEvalConstExpr("2 < 2 < 2", 0.0);
    checkEvalConstExpr("1 < 2 < 2", 0.0);
    checkEvalConstExpr("1 < 1 + 1 < 1 + 1 + 1", 1.0);
}

TEST(ExpressionAssembly, DoubleInequalityLte)
{
    checkEvalConstExpr("1 <= 2 <= 3", 1.0);
    checkEvalConstExpr("2 <= 2 <= 3", 1.0);
    checkEvalConstExpr("2 <= 2 <= 2", 1.0);
    checkEvalConstExpr("1 <= 2 <= 2", 1.0);
    checkEvalConstExpr("3 <= 2 <= 3", 0.0);
    checkEvalConstExpr("1 <= 4 <= 3", 0.0);
    checkEvalConstExpr("1 <= 2 <= 1", 0.0);
    checkEvalConstExpr("1 <= 1 + 1 - 1 <= 1 + 1 + 1 - 2", 1.0);
}

TEST(ExpressionAssembly, DoubleInequalityGt)
{
    checkEvalConstExpr("3 > 2 > 1", 1.0);
    checkEvalConstExpr("3 > 2 > 2", 0.0);
    checkEvalConstExpr("2 > 2 > 2", 0.0);
    checkEvalConstExpr("2 > 2 > 1", 0.0);
    checkEvalConstExpr("1 + 1 + 1 > 1 + 1 > 1", 1.0);
}

TEST(ExpressionAssembly, DoubleInequalityGte)
{
    checkEvalConstExpr("3 >= 2 >= 1", 1.0);
    checkEvalConstExpr("3 >= 2 >= 2", 1.0);
    checkEvalConstExpr("2 >= 2 >= 2", 1.0);
    checkEvalConstExpr("2 >= 2 >= 1", 1.0);
    checkEvalConstExpr("3 >= 2 >= 3", 0.0);
    checkEvalConstExpr("3 >= 4 >= 1", 0.0);
    checkEvalConstExpr("1 >= 2 >= 1", 0.0);
    checkEvalConstExpr("1 + 1 + 1 - 2 >= 1 + 1 - 1 >= 1", 1.0);
}

TEST(ExpressionAssembly, DoubleInequalityLtLte)
{
    checkEvalConstExpr("1 < 2 <= 3", 1.0);
    checkEvalConstExpr("1 < 2 <= 2", 1.0);
    checkEvalConstExpr("1 < 2 <= 1", 0.0);
    checkEvalConstExpr("2 < 2 <= 2", 0.0);
}

TEST(ExpressionAssembly, DoubleInequalityGtGte)
{
    checkEvalConstExpr("3 > 2 >= 1", 1.0);
    checkEvalConstExpr("2 > 2 >= 1", 0.0);
    checkEvalConstExpr("1 > 2 >= 1", 0.0);
    checkEvalConstExpr("2 > 2 >= 2", 0.0);
}

TEST(ExpressionAssembly, DoubleInequalityOpposingComparisons)
{
    checkEvalConstExpr("3 > 2 < 4", 1.0);
    checkEvalConstExpr("3 > 2 < 2", 0.0);
    checkEvalConstExpr("3 > 2 <= 4", 1.0);
    checkEvalConstExpr("2 > 2 < 4", 0.0);
    checkEvalConstExpr("2 >= 2 < 4", 1.0);
}

TEST(ExpressionAssembly, TripleInequality)
{
    checkEvalConstExpr("1 < 2 < 3 < 4", 1.0);
    checkEvalConstExpr("1 < 1 < 3 < 4", 0.0);
    checkEvalConstExpr("1 <= 1 < 3 < 4", 1.0);
    checkEvalConstExpr("1 < 2 < 2 < 4", 0.0);
    checkEvalConstExpr("1 < 2 <= 2 < 4", 1.0);
    checkEvalConstExpr("1 < 2 < 3 < 3", 0.0);
    checkEvalConstExpr("1 < 2 < 3 <= 3", 1.0);
}

TEST(ExpressionAssembly, OnlyElement)
{
    // Parse expression.
    PARSE_EXPR("foo");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to 0, the initial value of element `foo`.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to a new value and re-evaluate expression.
    elemFoo.write(3);
    CHECK_EQUAL(3.0, root->evaluate());
}

TEST(ExpressionAssembly, MultipleElements)
{
    // Parse expression.
    PARSE_EXPR("(foo + bar) * baz + 1");

    // Create element bindings.
    I32 foo = 0;
    I32 bar = 0;
    I32 baz = 0;
    Element<I32> elemFoo(foo);
    Element<I32> elemBar(bar);
    Element<I32> elemBaz(baz);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo},
        {"bar", &elemBar},
        {"baz", &elemBaz}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 1.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(1.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    elemFoo.write(3);
    elemBar.write(-10);
    elemBaz.write(4);
    CHECK_EQUAL(-27.0, root->evaluate());
}

TEST(ExpressionAssembly, AllElementTypes)
{
    // Parse expression.
    PARSE_EXPR("a + b + c + d + e + f + g + h + i + j + k");

    // Create element bindings.
    I8 a = 0;
    I16 b = 0;
    I32 c = 0;
    I64 d = 0;
    U8 e = 0;
    U16 f = 0;
    U32 g = 0;
    U64 h = 0;
    F32 i = 0;
    F64 j = 0;
    bool k = 0;
    Element<I8> elemA(a);
    Element<I16> elemB(b);
    Element<I32> elemC(c);
    Element<I64> elemD(d);
    Element<U8> elemE(e);
    Element<U16> elemF(f);
    Element<U32> elemG(g);
    Element<U64> elemH(h);
    Element<F32> elemI(i);
    Element<F64> elemJ(j);
    Element<bool> elemK(k);
    const Map<String, IElement*> bindings =
    {
        {"a", &elemA},
        {"b", &elemB},
        {"c", &elemC},
        {"d", &elemD},
        {"e", &elemE},
        {"f", &elemF},
        {"g", &elemG},
        {"h", &elemH},
        {"i", &elemI},
        {"j", &elemJ},
        {"k", &elemK}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 0.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    elemA.write(11);
    elemB.write(10);
    elemC.write(9);
    elemD.write(8);
    elemE.write(7);
    elemF.write(6);
    elemG.write(5);
    elemH.write(4);
    elemI.write(3.0f);
    elemJ.write(2.0);
    elemK.write(true);
    CHECK_EQUAL(66.0, root->evaluate());
}

TEST(ExpressionAssembly, RollAvgFunction)
{
    // Parse expression.
    PARSE_EXPR("ROLL_AVG(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to 2 and update stats. Rolling average becomes 2.
    elemFoo.write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 4 and update stats. Rolling average becomes 3.
    elemFoo.write(4);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 6 and update stats. Rolling average becomes 5 since the
    // oldest value (2) falls out of the window.
    elemFoo.write(6);
    stats.update();
    CHECK_EQUAL(5.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMedianFunction)
{
    // Parse expression.
    PARSE_EXPR("ROLL_MEDIAN(foo, 3)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to 2 and update stats. Rolling median becomes 2.
    elemFoo.write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 4 and update stats. Rolling median becomes 3.
    elemFoo.write(4);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 6 and update stats. Rolling median becomes 4.
    elemFoo.write(6);
    stats.update();
    CHECK_EQUAL(4.0, root->evaluate());

    // Set `foo` to 7 and update stats. Rolling median becomes 6 since the
    // oldest value (2) falls out of the window.
    elemFoo.write(7);
    stats.update();
    CHECK_EQUAL(6.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMinFunction)
{
    // Parse expression.
    PARSE_EXPR("ROLL_MIN(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to -3 and update stats. Rolling min becomes -3.
    elemFoo.write(-3);
    stats.update();
    CHECK_EQUAL(-3.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling min stays -3.
    elemFoo.write(1);
    stats.update();
    CHECK_EQUAL(-3.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling min becomes 1 since the oldest
    // value (-3) falls out of the window.
    elemFoo.write(2);
    stats.update();
    CHECK_EQUAL(1.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMaxFunction)
{
    // Parse expression.
    PARSE_EXPR("ROLL_MAX(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to 3 and update stats. Rolling max becomes 3.
    elemFoo.write(3);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling max stays 3.
    elemFoo.write(1);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling max becomes 2 since the oldest
    // value (3) falls out of the window.
    elemFoo.write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());
}

TEST(ExpressionAssembly, RollRangeFunction)
{
    // Parse expression.
    PARSE_EXPR("ROLL_RANGE(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to 3 and update stats. Rolling range stays 0 since
    // there's only 1 value in the window.
    elemFoo.write(3);
    stats.update();
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling range becomes 2.
    elemFoo.write(1);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 5 and update stats. Rolling range becomes 4 since the oldest
    // value (3) falls out of the window.
    elemFoo.write(5);
    stats.update();
    CHECK_EQUAL(4.0, root->evaluate());
}

TEST(ExpressionAssembly, StatsFunctionExpressionArgs)
{
    // Parse expression.
    PARSE_EXPR("ROLL_MIN(foo + 1, bar * -1)");

    // Create element bindings.
    I32 foo = 0;
    I32 bar = 0;
    Element<I32> elemFoo(foo);
    Element<I32> elemBar(bar);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo},
        {"bar", &elemBar}
    };

    // Set element `bar` to -2. This causes the function call to use a window
    // size of 2.
    elemBar.write(-2);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Set `bar` to something else. This doesn't affect the expression since the
    // `ROLL_MIN` window size is evaluated at compile time.
    elemBar.write(10);

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set element `foo` to -3 and update stats. Rolling min becomes -2.
    elemFoo.write(-3);
    stats.update();
    CHECK_EQUAL(-2.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling min stays -2.
    elemFoo.write(1);
    stats.update();
    CHECK_EQUAL(-2.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling min becomes 2 since the oldest
    // value (-2) falls out of the window.
    elemFoo.write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(ExpressionAssemblyErrors)
{
};

TEST(ExpressionAssemblyErrors, UnknownElement)
{
    PARSE_EXPR("foo");
    checkCompileError(exprParse, {}, E_EXA_ELEM, 1, 1);
}

TEST(ExpressionAssemblyErrors, OutOfRangeNumber)
{
    PARSE_EXPR("1 + 999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999");
    checkCompileError(exprParse, {}, E_EXA_OVFL, 1, 5);
}

TEST(ExpressionAssemblyErrors, StatsFunctionArity)
{
    PARSE_EXPR("ROLL_AVG(1)");
    checkCompileError(exprParse, {}, E_EXA_ARITY, 1, 1);
}

TEST(ExpressionAssemblyErrors, StatsFunctionErrorInArg1)
{
    PARSE_EXPR("ROLL_AVG(foo, 4)");
    checkCompileError(exprParse, {}, E_EXA_ELEM, 1, 10);
}

TEST(ExpressionAssemblyErrors, StatsFunctionErrorInArg2)
{
    PARSE_EXPR("ROLL_AVG(4, foo)");
    checkCompileError(exprParse, {}, E_EXA_ELEM, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionZeroWindowSize)
{
    PARSE_EXPR("ROLL_AVG(4, 0)");
    checkCompileError(exprParse, {}, E_EXA_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNegativeWindowSize)
{
    PARSE_EXPR("ROLL_AVG(4, -1)");
    checkCompileError(exprParse, {}, E_EXA_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNonIntegerWindowSize)
{
    PARSE_EXPR("ROLL_AVG(4, 1.5)");
    checkCompileError(exprParse, {}, E_EXA_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNaNWindowSize)
{
    PARSE_EXPR("ROLL_AVG(4, 0 / 0)");
    checkCompileError(exprParse, {}, E_EXA_WIN, 1, 15);
}

TEST(ExpressionAssemblyErrors, StatsFunctionWindowTooBig)
{
    PARSE_EXPR("ROLL_AVG(4, 100001)");
    checkCompileError(exprParse, {}, E_EXA_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, UnknownFunction)
{
    PARSE_EXPR("FOO()");
    checkCompileError(exprParse, {}, E_EXA_FUNC, 1, 1);
}

TEST(ExpressionAssemblyErrors, NullElementInBindings)
{
    PARSE_EXPR("foo");
    const Map<String, IElement*> bindings = {{"foo", nullptr}};
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_ERROR(E_EXA_ELEM_NULL,
                ExpressionAssembly::compile(exprParse,
                                            bindings,
                                            ElementType::FLOAT64,
                                            exprAsm,
                                            nullptr));
}

TEST(ExpressionAssemblyErrors, NullParse)
{
    const Ref<const ExpressionParse> exprParse;
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_ERROR(E_EXA_NULL, ExpressionAssembly::compile(exprParse,
                                                        {},
                                                        ElementType::FLOAT64,
                                                        exprAsm,
                                                        nullptr));
    CHECK_TRUE(exprAsm == nullptr);
}
