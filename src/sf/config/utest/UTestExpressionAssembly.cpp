#include <cstring>

#include "sf/config/ExpressionAssembly.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void setup(const char* const kExprSrc,
                  const char* const kSvSrc,
                  Ref<const ExpressionParse>& kExprParse,
                  Ref<const StateVectorAssembly>& kSvAsm,
                  Ref<StateVector>& kSv)
{
    // Parse expression.
    Vec<Token> exprToks;
    std::stringstream exprSs(kExprSrc);
    CHECK_SUCCESS(Tokenizer::tokenize(exprSs, exprToks, nullptr));
    TokenIterator exprIt(exprToks.begin(), exprToks.end());
    CHECK_SUCCESS(ExpressionParse::parse(exprIt, kExprParse, nullptr));

    // Compile state vector.
    if (std::strlen(kSvSrc) > 0)
    {
        std::stringstream svSs(kSvSrc);
        CHECK_SUCCESS(StateVectorAssembly::compile(svSs, kSvAsm, nullptr));
        kSv = kSvAsm->get();
    }
}

static void checkEvalConstExpr(const char* const kExprSrc, const F64 kExpectVal)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    setup(kExprSrc, "", exprParse, svAsm, sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
                              const Ref<StateVector> kSv,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Got expected return code from compiler.
    Ref<const ExpressionAssembly> exprAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, ExpressionAssembly::compile(kExprParse,
                                                  {kSv},
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
}

///////////////////////////////// Usage Tests //////////////////////////////////

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
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("foo",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to 0, the initial value of element `foo`.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to a new value and re-evaluate expression.
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(3);
    CHECK_EQUAL(3.0, root->evaluate());
}

TEST(ExpressionAssembly, MultipleElements)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("(foo + bar) * baz + 1",
            "[Foo]\n"
            "I32 foo\n"
            "I32 bar\n"
            "I32 baz\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 1.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(1.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    Element<I32>* elemFoo = nullptr;
    Element<I32>* elemBar = nullptr;
    Element<I32>* elemBaz = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    CHECK_SUCCESS(sv->getElement("bar", elemBar));
    CHECK_SUCCESS(sv->getElement("baz", elemBaz));
    elemFoo->write(3);
    elemBar->write(-10);
    elemBaz->write(4);
    CHECK_EQUAL(-27.0, root->evaluate());
}

TEST(ExpressionAssembly, AllElementTypes)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("a + b + c + d + e + f + g + h + i + j + k",
            "[Foo]\n"
            "I8 a\n"
            "I16 b\n"
            "I32 c\n"
            "I64 d\n"
            "U8 e\n"
            "U16 f\n"
            "U32 g\n"
            "U64 h\n"
            "F32 i\n"
            "F64 j\n"
            "BOOL k\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 0.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    IExprNode<F64>* const root =
        static_cast<IExprNode<F64>* const>(exprAsm->root().get());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    Element<I8>* elemA = nullptr;
    Element<I16>* elemB = nullptr;
    Element<I32>* elemC = nullptr;
    Element<I64>* elemD = nullptr;
    Element<U8>* elemE = nullptr;
    Element<U16>* elemF = nullptr;
    Element<U32>* elemG = nullptr;
    Element<U64>* elemH = nullptr;
    Element<F32>* elemI = nullptr;
    Element<F64>* elemJ = nullptr;
    Element<bool>* elemK = nullptr;
    CHECK_SUCCESS(sv->getElement("a", elemA));
    CHECK_SUCCESS(sv->getElement("b", elemB));
    CHECK_SUCCESS(sv->getElement("c", elemC));
    CHECK_SUCCESS(sv->getElement("d", elemD));
    CHECK_SUCCESS(sv->getElement("e", elemE));
    CHECK_SUCCESS(sv->getElement("f", elemF));
    CHECK_SUCCESS(sv->getElement("g", elemG));
    CHECK_SUCCESS(sv->getElement("h", elemH));
    CHECK_SUCCESS(sv->getElement("i", elemI));
    CHECK_SUCCESS(sv->getElement("j", elemJ));
    CHECK_SUCCESS(sv->getElement("k", elemK));
    elemA->write(1);
    elemB->write(1);
    elemC->write(1);
    elemD->write(1);
    elemE->write(1);
    elemF->write(1);
    elemG->write(1);
    elemH->write(1);
    elemI->write(1.0f);
    elemJ->write(1.0);
    elemK->write(true);
    CHECK_EQUAL(11.0, root->evaluate());
}

TEST(ExpressionAssembly, RollAvgFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(foo, 2)",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 4 and update stats. Rolling average becomes 3.
    elemFoo->write(4);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 6 and update stats. Rolling average becomes 5 since the
    // oldest value (2) falls out of the window.
    elemFoo->write(6);
    stats.update();
    CHECK_EQUAL(5.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMedianFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_MEDIAN(foo, 3)",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 4 and update stats. Rolling median becomes 3.
    elemFoo->write(4);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 6 and update stats. Rolling median becomes 4.
    elemFoo->write(6);
    stats.update();
    CHECK_EQUAL(4.0, root->evaluate());

    // Set `foo` to 7 and update stats. Rolling median becomes 6 since the
    // oldest value (2) falls out of the window.
    elemFoo->write(7);
    stats.update();
    CHECK_EQUAL(6.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMinFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_MIN(foo, 2)",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(-3);
    stats.update();
    CHECK_EQUAL(-3.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling min stays -3.
    elemFoo->write(1);
    stats.update();
    CHECK_EQUAL(-3.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling min becomes 1 since the oldest
    // value (-3) falls out of the window.
    elemFoo->write(2);
    stats.update();
    CHECK_EQUAL(1.0, root->evaluate());
}

TEST(ExpressionAssembly, RollMaxFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_MAX(foo, 2)",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(3);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling max stays 3.
    elemFoo->write(1);
    stats.update();
    CHECK_EQUAL(3.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling max becomes 2 since the oldest
    // value (3) falls out of the window.
    elemFoo->write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());
}

TEST(ExpressionAssembly, RollRangeFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_RANGE(foo, 2)",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(3);
    stats.update();
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling range becomes 2.
    elemFoo->write(1);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());

    // Set `foo` to 5 and update stats. Rolling range becomes 4 since the oldest
    // value (3) falls out of the window.
    elemFoo->write(5);
    stats.update();
    CHECK_EQUAL(4.0, root->evaluate());
}

TEST(ExpressionAssembly, StatsFunctionExpressionArgs)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_MIN(foo + 1, bar * -1)",
            "[Foo]\n"
            "I32 foo\n"
            "I32 bar\n",
            exprParse,
            svAsm,
            sv);

    // Set element `bar` to -2. This causes the `ROLL_MIN` call to use a window
    // size of 2.
    Element<I32>* elemBar = nullptr;
    CHECK_SUCCESS(sv->getElement("bar", elemBar));
    elemBar->write(-2);

    // Compile expression.
    Ref<const ExpressionAssembly> exprAsm;
    CHECK_SUCCESS(ExpressionAssembly::compile(exprParse,
                                              {sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Set `bar` to something else. This doesn't affect the expression since the
    // `ROLL_MIN` window size is evaluated at compile time.
    elemBar->write(10);

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
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv->getElement("foo", elemFoo));
    elemFoo->write(-3);
    stats.update();
    CHECK_EQUAL(-2.0, root->evaluate());

    // Set `foo` to 1 and update stats. Rolling min stays -2.
    elemFoo->write(1);
    stats.update();
    CHECK_EQUAL(-2.0, root->evaluate());

    // Set `foo` to 2 and update stats. Rolling min becomes 2 since the oldest
    // value (-2) falls out of the window.
    elemFoo->write(2);
    stats.update();
    CHECK_EQUAL(2.0, root->evaluate());
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(ExpressionAssemblyErrors)
{
};

TEST(ExpressionAssemblyErrors, UnknownElement)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("foo", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_ELEM, 1, 1);
}

TEST(ExpressionAssemblyErrors, OutOfRangeNumber)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("1 + 9999999999999999999999999999999999999999999999999999999999999"
            "99999999999999999999999999999999999999999999999999999999999999999"
            "99999999999999999999999999999999999999999999999999999999999999999"
            "99999999999999999999999999999999999999999999999999999999999999999"
            "99999999999999999999999999999999999999999999999999999999999999999"
            "99999999999999999999999999999999999999999999999999999999999999999",
            "",
            exprParse,
            svAsm,
            sv);
    checkCompileError(exprParse, sv, E_EXC_OVFL, 1, 5);
}

TEST(ExpressionAssemblyErrors, StatsFunctionArity)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(1)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_ARITY, 1, 1);
}

TEST(ExpressionAssemblyErrors, StatsFunctionErrorInArg1)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(foo, 4)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_ELEM, 1, 10);
}

TEST(ExpressionAssemblyErrors, StatsFunctionErrorInArg2)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, foo)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_ELEM, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionZeroWindowSize)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, 0)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNegativeWindowSize)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, -1)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNonIntegerWindowSize)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, 1.5)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, StatsFunctionNaNWindowSize)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, 1 / 0)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_WIN, 1, 15);
}

TEST(ExpressionAssemblyErrors, StatsFunctionWindowTooBig)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("ROLL_AVG(4, 100001)", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_WIN, 1, 13);
}

TEST(ExpressionAssemblyErrors, UnknownFunction)
{
    Ref<const ExpressionParse> exprParse;
    Ref<const StateVectorAssembly> svAsm;
    Ref<StateVector> sv;
    ::setup("FOO()", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_FUNC, 1, 1);
}
