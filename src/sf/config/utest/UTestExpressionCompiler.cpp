#include <cstring>

#include "sf/config/ExpressionCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void setup(const char* const kExprSrc,
                  const char* const kSvSrc,
                  Ref<const ExpressionParser::Parse>& kExprParse,
                  Ref<const StateVectorCompiler::Assembly>& kSvAsm,
                  StateVector& kSv)
{
    // Parse expression.
    Vec<Token> exprToks;
    std::stringstream exprSs(kExprSrc);
    CHECK_SUCCESS(Tokenizer::tokenize(exprSs, exprToks, nullptr));
    TokenIterator exprIt(exprToks.begin(), exprToks.end());
    CHECK_SUCCESS(ExpressionParser::parse(exprIt, kExprParse, nullptr));

    // Compile state vector.
    if (std::strlen(kSvSrc) > 0)
    {
        std::stringstream svSs(kSvSrc);
        CHECK_SUCCESS(StateVectorCompiler::compile(svSs, kSvAsm, nullptr));
        CHECK_SUCCESS(StateVector::create(kSvAsm->getConfig(), kSv));
    }
}

static void checkEvalConstExpr(const char* const kExprSrc, const F64 kExpectVal)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    setup(kExprSrc, "", exprParse, svAsm, sv);

    // Compile expression.
    Ref<const ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              {&sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    CHECK_EQUAL(kExpectVal, root->evaluate());
}

static void checkCompileError(
    const Ref<const ExpressionParser::Parse> kExprParse,
    StateVector& kSv,
    const Result kRes,
    const I32 kLineNum,
    const I32 kColNum)
{
    // Got expected return code from compiler.
    Ref<const ExpressionCompiler::Assembly> exprAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, ExpressionCompiler::compile(kExprParse,
                                                  {&kSv},
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
    checkEvalConstExpr("NOT FALSE", 1.0);
    checkEvalConstExpr("NOT TRUE", 0.0);
    checkEvalConstExpr("NOT NOT FALSE", 0.0);
    checkEvalConstExpr("NOT NOT NOT FALSE", 1.0);
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
    checkEvalConstExpr("FALSE AND FALSE", 0.0);
    checkEvalConstExpr("FALSE AND TRUE", 0.0);
    checkEvalConstExpr("TRUE AND FALSE", 0.0);
    checkEvalConstExpr("TRUE AND TRUE", 1.0);
}

TEST(ExpressionCompiler, Or)
{
    checkEvalConstExpr("FALSE OR FALSE", 0.0);
    checkEvalConstExpr("FALSE OR TRUE", 1.0);
    checkEvalConstExpr("TRUE OR FALSE", 1.0);
    checkEvalConstExpr("TRUE OR TRUE", 1.0);
}

TEST(ExpressionCompiler, ComplexLogic)
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
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("foo",
            "[Foo]\n"
            "I32 foo\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              {&sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression evaluates to 0, the initial value of element `foo`.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    CHECK_EQUAL(0.0, root->evaluate());

    // Set `foo` to a new value and re-evaluate expression.
    Element<I32>* elemFoo = nullptr;
    CHECK_SUCCESS(sv.getElement("foo", elemFoo));
    elemFoo->write(3);
    CHECK_EQUAL(3.0, root->evaluate());
}

TEST(ExpressionCompiler, MultipleElements)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("(foo + bar) * baz + 1",
            "[Foo]\n"
            "I32 foo\n"
            "I32 bar\n"
            "I32 baz\n",
            exprParse,
            svAsm,
            sv);

    // Compile expression.
    Ref<const ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              {&sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 1.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    CHECK_EQUAL(1.0, root->evaluate());

    // Set elements to new values and re-evaluate expression.
    Element<I32>* elemFoo = nullptr;
    Element<I32>* elemBar = nullptr;
    Element<I32>* elemBaz = nullptr;
    CHECK_SUCCESS(sv.getElement("foo", elemFoo));
    CHECK_SUCCESS(sv.getElement("bar", elemBar));
    CHECK_SUCCESS(sv.getElement("baz", elemBaz));
    elemFoo->write(3);
    elemBar->write(-10);
    elemBaz->write(4);
    CHECK_EQUAL(-27.0, root->evaluate());
}

TEST(ExpressionCompiler, AllElementTypes)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
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
    Ref<const ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(ExpressionCompiler::compile(exprParse,
                                              {&sv},
                                              ElementType::FLOAT64,
                                              exprAsm,
                                              nullptr));

    // Expression initially evaluates to 0.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
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
    CHECK_SUCCESS(sv.getElement("a", elemA));
    CHECK_SUCCESS(sv.getElement("b", elemB));
    CHECK_SUCCESS(sv.getElement("c", elemC));
    CHECK_SUCCESS(sv.getElement("d", elemD));
    CHECK_SUCCESS(sv.getElement("e", elemE));
    CHECK_SUCCESS(sv.getElement("f", elemF));
    CHECK_SUCCESS(sv.getElement("g", elemG));
    CHECK_SUCCESS(sv.getElement("h", elemH));
    CHECK_SUCCESS(sv.getElement("i", elemI));
    CHECK_SUCCESS(sv.getElement("j", elemJ));
    CHECK_SUCCESS(sv.getElement("k", elemK));
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

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(ExpressionCompilerErrors)
{
};

TEST(ExpressionCompilerErrors, UnknownElement)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("foo", "", exprParse, svAsm, sv);
    checkCompileError(exprParse, sv, E_EXC_ELEM, 1, 1);
}

TEST(ExpressionCompilerErrors, InvalidNumber)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("1 + 1", "", exprParse, svAsm, sv);
    exprParse->left->data.str = "foo";
    checkCompileError(exprParse, sv, E_EXC_NUM, 1, 1);
}

TEST(ExpressionCompilerErrors, OutOfRangeNumber)
{
    Ref<const ExpressionParser::Parse> exprParse;
    Ref<const StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
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
