#include <cstring>

#include "sfa/sup/ExpressionCompiler.hpp"
#include "sfa/sup/StateVectorCompiler.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void setup(const char* const kExprSrc,
                  const char* const kSvSrc,
                  std::shared_ptr<ExpressionParser::Parse>& kExprParse,
                  std::shared_ptr<StateVectorCompiler::Assembly>& kSvAsm,
                  StateVector& kSv)
{
    // Parse expression.
    std::vector<Token> exprToks;
    std::stringstream exprSs(kExprSrc);
    CHECK_SUCCESS(ConfigTokenizer::tokenize(exprSs, exprToks, nullptr));
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

static void checkEvalConstExpr(const char* const kExprSrc,
                               const F64 kExpectVal)
{
    std::shared_ptr<ExpressionParser::Parse> exprParse;
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    setup(kExprSrc, "", exprParse, svAsm, sv);

    // Compile expression.
    std::shared_ptr<ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(
        ExpressionCompiler::compile(exprParse, true, sv, exprAsm, nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    CHECK_EQUAL(kExpectVal, root->evaluate());
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(ExpressionCompiler)
{
};

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
    checkEvalConstExpr("5 * ((3 * -3.14) * 9.81) * -1.62",
                       (5 * ((3 * -3.14) * 9.81) * -1.62));
}

TEST(ExpressionCompiler, Divide)
{
    checkEvalConstExpr("5 / 3", (5.0 / 3.0));
    checkEvalConstExpr("5 / 3 / -3.14 / 9.81 / -1.62",
                       (5.0 / 3 / -3.14 / 9.81 / -1.62));
    checkEvalConstExpr("5 / ((3 / -3.14) / 9.81) / -1.62",
                       (5.0 / ((3 / -3.14) / 9.81) / -1.62));
}

TEST(ExpressionCompiler, Add)
{
    checkEvalConstExpr("5 + 3", (5.0 + 3.0));
    checkEvalConstExpr("5 + 3 + -3.14 + 9.81 + -1.62",
                       (5 + 3 + -3.14 + 9.81 + -1.62));
    checkEvalConstExpr("5 + ((3 + -3.14) + 9.81) + -1.62",
                       (5 + ((3 + -3.14) + 9.81) + -1.62));
}

TEST(ExpressionCompiler, Subtract)
{
    checkEvalConstExpr("5 - 3", (5.0 - 3.0));
    checkEvalConstExpr("5 - 3 - -3.14 - 9.81 - -1.62",
                       (5 - 3 - -3.14 - 9.81 - -1.62));
    checkEvalConstExpr("5 - ((3 - -3.14) - 9.81) - -1.62",
                       (5 - ((3 - -3.14) - 9.81) - -1.62));
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
    const double expectVal =
        (true || !(false && true && !(false && !false)) || true && false
         || ((true && !true || false) || !!true) || false && true || false
         && (!false && true) && !(!(true || false) || (!false || true)));
    checkEvalConstExpr(
        "(TRUE OR !(FALSE AND TRUE AND !(FALSE AND !FALSE)) OR TRUE AND FALSE"
        " OR ((TRUE AND !TRUE OR FALSE) OR !!TRUE) OR FALSE AND TRUE OR FALSE"
        " AND (!FALSE AND TRUE) AND !(!(TRUE OR FALSE) OR (!FALSE OR TRUE)))",
        expectVal);
}
