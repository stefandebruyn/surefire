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

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(ExpressionCompiler)
{
};

TEST(ExpressionCompiler, SimpleAdd)
{
    std::shared_ptr<ExpressionParser::Parse> exprParse;
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("5 + 3", "", exprParse, svAsm, sv);

    // Compile expression.
    std::shared_ptr<ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(
        ExpressionCompiler::compile(exprParse, true, sv, exprAsm, nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    const double expectVal = (5.0 + 3.0);
    CHECK_EQUAL(expectVal, root->evaluate());
}

TEST(ExpressionCompiler, SimpleSubtract)
{
    std::shared_ptr<ExpressionParser::Parse> exprParse;
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("5 - 3", "", exprParse, svAsm, sv);

    // Compile expression.
    std::shared_ptr<ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(
        ExpressionCompiler::compile(exprParse, true, sv, exprAsm, nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    const double expectVal = (5.0 - 3.0);
    CHECK_EQUAL(expectVal, root->evaluate());
}

TEST(ExpressionCompiler, SimpleMultiply)
{
    std::shared_ptr<ExpressionParser::Parse> exprParse;
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("5 * 3", "", exprParse, svAsm, sv);

    // Compile expression.
    std::shared_ptr<ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(
        ExpressionCompiler::compile(exprParse, true, sv, exprAsm, nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    const double expectVal = (5.0 * 3.0);
    CHECK_EQUAL(expectVal, root->evaluate());
}

TEST(ExpressionCompiler, SimpleDivide)
{
    std::shared_ptr<ExpressionParser::Parse> exprParse;
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;
    StateVector sv;
    ::setup("5 / 3", "", exprParse, svAsm, sv);

    // Compile expression.
    std::shared_ptr<ExpressionCompiler::Assembly> exprAsm;
    CHECK_SUCCESS(
        ExpressionCompiler::compile(exprParse, true, sv, exprAsm, nullptr));

    // Expression evaluates to expected value.
    CHECK_EQUAL(ElementType::FLOAT64, exprAsm->root()->type());
    const IExprNode<F64>* const root =
        static_cast<const IExprNode<F64>* const>(exprAsm->root());
    const double expectVal = (5.0 / 3.0);
    CHECK_EQUAL(expectVal, root->evaluate());
}
