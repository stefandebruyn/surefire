#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/utest/UTest.hpp"

#define TOKENIZE(kSrc)                                                         \
    std::stringstream ss(kSrc);                                                \
    std::vector<Token> toks;                                                   \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toks, nullptr));               \
    TokenIterator it(toks.begin(), toks.end());

static ExpressionParser::Parse gParse;

static void deleteExpressionParse(ExpressionParser::Parse* const kParse)
{
    if (kParse == nullptr)
    {
        return;
    }

    deleteExpressionParse(kParse->left);
    deleteExpressionParse(kParse->right);

    delete kParse->left;
    delete kParse->right;
}

TEST_GROUP(ExpressionParser)
{
    void teardown()
    {
        deleteExpressionParse(&gParse);
    }
};

TEST(ExpressionParser, Doop)
{
    TOKENIZE("10 + 10");
    CHECK_SUCCESS(ExpressionParser::parse(it, gParse, nullptr));

    CHECK_TRUE(gParse.data == toks[1]);
    CHECK_TRUE(gParse.left != nullptr);
    CHECK_TRUE(gParse.right != nullptr);

    const ExpressionParser::Parse* node = gParse.left;
    CHECK_TRUE(node->data == toks[0]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);

    node = gParse.right;
    CHECK_TRUE(node->data == toks[2]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);
}
