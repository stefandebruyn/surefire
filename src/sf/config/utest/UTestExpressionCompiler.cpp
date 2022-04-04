#include <cstring>

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define PARSE_EXPR(kExprSrc)                                                   \
    TOKENIZE(kExprSrc);                                                        \
    Ref<const ExpressionParse> exprParse;                                      \
    CHECK_SUCCESS(ExpressionParser::parse(it, exprParse, nullptr));

static void checkEvalConstExpr(const char* const kExprSrc, const F64 kExpectVal)
{
    PARSE_EXPR(kExprSrc);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              {},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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
    CHECK_ERROR(kRes, ExpressionCompiler::compile(kExprParse,
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
    CHECK_ERROR(kRes, ExpressionCompiler::compile(kExprParse,
                                                  kBindings,
                                                  ElementType::FLOAT64,
                                                  exprAsm,
                                                  nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(ExpressionCompiler)
{
};

TEST(ExpressionCompiler, SimplePrecedence)
{
    checkEvalConstExpr("1 + 2 * 3", 7);
}

TEST(ExpressionCompiler, SimplePrecedenceWithParens)
{
    checkEvalConstExpr("(1 + 2) * 3", 9);
}

TEST(ExpressionCompiler, Not)
{
    checkEvalConstExpr("not false", 1.0);
    checkEvalConstExpr("not true", 0.0);
    checkEvalConstExpr("not not false", 0.0);
    checkEvalConstExpr("not not not false", 1.0);
}

TEST(ExpressionCompiler, Multiply)
{
    checkEvalConstExpr("5 * 3", (5 * 3));
    checkEvalConstExpr("5 * 3 * -3.14 * 9.81 * -1.62",
                       (5 * 3 * -3.14 * 9.81 * -1.62));
    checkEvalConstExpr("5 * (3 * (-3.14 * 9.81)) * -1.62",
                       (5 * (3 * (-3.14 * 9.81)) * -1.62));
}

TEST(ExpressionCompiler, Divide)
{
    checkEvalConstExpr("5 / 3", (5.0 / 3.0));
    checkEvalConstExpr("5 / 3 / -3.14 / 9.81 / -1.62",
                       (5.0 / 3 / -3.14 / 9.81 / -1.62));
    checkEvalConstExpr("5 / (3 / (-3.14 / 9.81)) / -1.62",
                       (5.0 / (3 / (-3.14 / 9.81)) / -1.62));
}

TEST(ExpressionCompiler, Add)
{
    checkEvalConstExpr("5 + 3", (5.0 + 3.0));
    checkEvalConstExpr("5 + 3 + -3.14 + 9.81 + -1.62",
                       (5 + 3 + -3.14 + 9.81 + -1.62));
    checkEvalConstExpr("5 + (3 + (-3.14 + 9.81)) + -1.62",
                       (5 + (3 + (-3.14 + 9.81)) + -1.62));
}

TEST(ExpressionCompiler, Subtract)
{
    checkEvalConstExpr("5 - 3", (5.0 - 3.0));
    checkEvalConstExpr("5 - 3 - -3.14 - 9.81 - -1.62",
                       (5 - 3 - -3.14 - 9.81 - -1.62));
    checkEvalConstExpr("5 - (3 - (-3.14 - 9.81)) - -1.62",
                       (5 - (3 - (-3.14 - 9.81)) - -1.62));
}

TEST(ExpressionCompiler, ComplexArithmetic)
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

TEST(ExpressionCompiler, LessThan)
{
    checkEvalConstExpr("3 < 5", 1.0);
    checkEvalConstExpr("5 < 3", 0.0);
    checkEvalConstExpr("5 < 5", 0.0);
}

TEST(ExpressionCompiler, LessThanEqual)
{
    checkEvalConstExpr("3 <= 5", 1.0);
    checkEvalConstExpr("5 <= 3", 0.0);
    checkEvalConstExpr("5 <= 5", 1.0);
}

TEST(ExpressionCompiler, GreaterThan)
{
    checkEvalConstExpr("5 > 3", 1.0);
    checkEvalConstExpr("3 > 5", 0.0);
    checkEvalConstExpr("5 > 5", 0.0);
}

TEST(ExpressionCompiler, GreaterThanEqual)
{
    checkEvalConstExpr("5 >= 3", 1.0);
    checkEvalConstExpr("3 >= 5", 0.0);
    checkEvalConstExpr("5 >= 5", 1.0);
}

TEST(ExpressionCompiler, Equal)
{
    checkEvalConstExpr("5 == 5", 1.0);
    checkEvalConstExpr("3 == 5", 0.0);
}

TEST(ExpressionCompiler, NotEqual)
{
    checkEvalConstExpr("3 != 5", 1.0);
    checkEvalConstExpr("5 != 5", 0.0);
}

TEST(ExpressionCompiler, And)
{
    checkEvalConstExpr("false and false", 0.0);
    checkEvalConstExpr("false and true", 0.0);
    checkEvalConstExpr("true and false", 0.0);
    checkEvalConstExpr("true and true", 1.0);
}

TEST(ExpressionCompiler, Or)
{
    checkEvalConstExpr("false or false", 0.0);
    checkEvalConstExpr("false or true", 1.0);
    checkEvalConstExpr("true or false", 1.0);
    checkEvalConstExpr("true or true", 1.0);
}

TEST(ExpressionCompiler, ComplexLogic)
{
    const bool expectVal =
        (true || !(false && true && !(false && !false)) || true && false
         || ((true && !true || false) || !!true) || false && true || false
         && (!false && true) && !(!(true || false) || (!false || true)));
    checkEvalConstExpr(
        "(true or !(false and true and !(false and !false)) or true and false"
        " or ((true and !true or false) or !!true) or false and true or false"
        " and (!false and true) and !(!(true or false) or (!false or true)))",
        expectVal);
}

TEST(ExpressionCompiler, MixedArithmeticAndLogic)
{
    checkEvalConstExpr("(4 + 6) / 2 == (100 - 120) / (4 * -1)", 1.0);
}

TEST(ExpressionCompiler, DoubleInequalityLt)
{
    checkEvalConstExpr("1 < 2 < 3", 1.0);
    checkEvalConstExpr("2 < 2 < 3", 0.0);
    checkEvalConstExpr("2 < 2 < 2", 0.0);
    checkEvalConstExpr("1 < 2 < 2", 0.0);
    checkEvalConstExpr("1 < 1 + 1 < 1 + 1 + 1", 1.0);
}

TEST(ExpressionCompiler, DoubleInequalityLte)
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

TEST(ExpressionCompiler, DoubleInequalityGt)
{
    checkEvalConstExpr("3 > 2 > 1", 1.0);
    checkEvalConstExpr("3 > 2 > 2", 0.0);
    checkEvalConstExpr("2 > 2 > 2", 0.0);
    checkEvalConstExpr("2 > 2 > 1", 0.0);
    checkEvalConstExpr("1 + 1 + 1 > 1 + 1 > 1", 1.0);
}

TEST(ExpressionCompiler, DoubleInequalityGte)
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

TEST(ExpressionCompiler, DoubleInequalityLtLte)
{
    checkEvalConstExpr("1 < 2 <= 3", 1.0);
    checkEvalConstExpr("1 < 2 <= 2", 1.0);
    checkEvalConstExpr("1 < 2 <= 1", 0.0);
    checkEvalConstExpr("2 < 2 <= 2", 0.0);
}

TEST(ExpressionCompiler, DoubleInequalityGtGte)
{
    checkEvalConstExpr("3 > 2 >= 1", 1.0);
    checkEvalConstExpr("2 > 2 >= 1", 0.0);
    checkEvalConstExpr("1 > 2 >= 1", 0.0);
    checkEvalConstExpr("2 > 2 >= 2", 0.0);
}

TEST(ExpressionCompiler, DoubleInequalityOpposingComparisons)
{
    checkEvalConstExpr("3 > 2 < 4", 1.0);
    checkEvalConstExpr("3 > 2 < 2", 0.0);
    checkEvalConstExpr("3 > 2 <= 4", 1.0);
    checkEvalConstExpr("2 > 2 < 4", 0.0);
    checkEvalConstExpr("2 >= 2 < 4", 1.0);
}

TEST(ExpressionCompiler, TripleInequality)
{
    checkEvalConstExpr("1 < 2 < 3 < 4", 1.0);
    checkEvalConstExpr("1 < 1 < 3 < 4", 0.0);
    checkEvalConstExpr("1 <= 1 < 3 < 4", 1.0);
    checkEvalConstExpr("1 < 2 < 2 < 4", 0.0);
    checkEvalConstExpr("1 < 2 <= 2 < 4", 1.0);
    checkEvalConstExpr("1 < 2 < 3 < 3", 0.0);
    checkEvalConstExpr("1 < 2 < 3 <= 3", 1.0);
}

TEST(ExpressionCompiler, OnlyElement)
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
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to 0, the initial value of element `foo`.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to a new value and re-evaluate expression.
    elemFoo.write(3);
    CHECK_EQUAL(3.0, root->evaluate());
}

TEST(ExpressionCompiler, MultipleElements)
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
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 1.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
    CHECK_EQUAL(1.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    elemFoo.write(3);
    elemBar.write(-10);
    elemBaz.write(4);
    CHECK_EQUAL(-27.0, root->evaluate());
}

TEST(ExpressionCompiler, AllElementTypes)
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
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 0.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, RollAvgFunction)
{
    // Parse expression.
    PARSE_EXPR("roll_avg(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
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
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, RollMedianFunction)
{
    // Parse expression.
    PARSE_EXPR("roll_median(foo, 3)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
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
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, RollMinFunction)
{
    // Parse expression.
    PARSE_EXPR("roll_min(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
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
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, RollMaxFunction)
{
    // Parse expression.
    PARSE_EXPR("roll_max(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
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
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, RollRangeFunction)
{
    // Parse expression.
    PARSE_EXPR("roll_range(foo, 2)");

    // Create element bindings.
    I32 foo = 0;
    Element<I32> elemFoo(foo);
    const Map<String, IElement*> bindings =
    {
        {"foo", &elemFoo}
    };

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
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
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST(ExpressionCompiler, StatsFunctionExpressionArgs)
{
    // Parse expression.
    PARSE_EXPR("roll_min(foo + 1, bar * -1)");

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
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              bindings,
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Set `bar` to something else. This doesn't affect the expression since the
    // `roll_min` window size is evaluated at compile time.
    elemBar.write(10);

    // Get expression stats used by function.
    const Vec<Ref<IExpressionStats>> statsVec = exprAsm->stats();
    CHECK_EQUAL(1, statsVec.size());
    IExpressionStats& stats = *statsVec[0];

    // Expression initially evaluates to 0 since stats have not been updated.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        dynamic_cast<IExprNode<F64>*>(exprAsm->root().get());
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

TEST_GROUP(ExpressionCompilerErrors)
{
};

TEST(ExpressionCompilerErrors, UnknownElement)
{
    PARSE_EXPR("foo");
    checkCompileError(exprParse, {}, E_EXC_ELEM, 1, 1);
}

TEST(ExpressionCompilerErrors, OutOfRangeNumber)
{
    PARSE_EXPR("1 + 999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999"
               "9999999999999999999999999999999999999999999999999999999999999");
    checkCompileError(exprParse, {}, E_EXC_OVFL, 1, 5);
}

TEST(ExpressionCompilerErrors, StatsFunctionArity)
{
    PARSE_EXPR("roll_avg(1)");
    checkCompileError(exprParse, {}, E_EXC_ARITY, 1, 1);
}

TEST(ExpressionCompilerErrors, StatsFunctionErrorInArg1)
{
    PARSE_EXPR("roll_avg(foo, 4)");
    checkCompileError(exprParse, {}, E_EXC_ELEM, 1, 10);
}

TEST(ExpressionCompilerErrors, StatsFunctionErrorInArg2)
{
    PARSE_EXPR("roll_avg(4, foo)");
    checkCompileError(exprParse, {}, E_EXC_ELEM, 1, 13);
}

TEST(ExpressionCompilerErrors, StatsFunctionZeroWindowSize)
{
    PARSE_EXPR("roll_avg(4, 0)");
    checkCompileError(exprParse, {}, E_EXC_WIN, 1, 13);
}

TEST(ExpressionCompilerErrors, StatsFunctionNegativeWindowSize)
{
    PARSE_EXPR("roll_avg(4, -1)");
    checkCompileError(exprParse, {}, E_EXC_WIN, 1, 13);
}

TEST(ExpressionCompilerErrors, StatsFunctionNonIntegerWindowSize)
{
    PARSE_EXPR("roll_avg(4, 1.5)");
    checkCompileError(exprParse, {}, E_EXC_WIN, 1, 13);
}

TEST(ExpressionCompilerErrors, StatsFunctionNaNWindowSize)
{
    PARSE_EXPR("roll_avg(4, 0 / 0)");
    checkCompileError(exprParse, {}, E_EXC_WIN, 1, 15);
}

TEST(ExpressionCompilerErrors, StatsFunctionWindowTooBig)
{
    PARSE_EXPR("roll_avg(4, 100001)");
    checkCompileError(exprParse, {}, E_EXC_WIN, 1, 13);
}

TEST(ExpressionCompilerErrors, UnknownFunction)
{
    PARSE_EXPR("FOO()");
    checkCompileError(exprParse, {}, E_EXC_FUNC, 1, 1);
}

TEST(ExpressionCompilerErrors, NullElementInBindings)
{
    PARSE_EXPR("foo");
    const Map<String, IElement*> bindings = {{"foo", nullptr}};
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_ERROR(E_EXC_ELEM_NULL,
                ExpressionCompiler::compile(exprParse,
                                            bindings,
                                            ElementType::FLOAT64,
                                            exprAsm,
                                            nullptr));
}

TEST(ExpressionCompilerErrors, NullParse)
{
    const Ref<const ExpressionParse> exprParse;
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_ERROR(E_EXC_NULL, ExpressionCompiler::compile(exprParse,
                                                        {},
                                                        ElementType::FLOAT64,
                                                        exprAsm,
                                                        nullptr));
    CHECK_TRUE(exprAsm == nullptr);
}
