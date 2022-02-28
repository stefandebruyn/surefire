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
    void setup()
    {
        gParse = {};
    }

    void teardown()
    {
        deleteExpressionParse(&gParse);
    }
};

TEST(ExpressionParser, OneConstant)
{
    TOKENIZE("10");
    CHECK_SUCCESS(ExpressionParser::parse(it, gParse, nullptr));
    CHECK_TRUE(gParse.data == toks[0]);
    POINTERS_EQUAL(nullptr, gParse.left);
    POINTERS_EQUAL(nullptr, gParse.right);
}

TEST(ExpressionParser, OneVariable)
{
    TOKENIZE("foo");
    CHECK_SUCCESS(ExpressionParser::parse(it, gParse, nullptr));
    CHECK_TRUE(gParse.data == toks[0]);
    POINTERS_EQUAL(nullptr, gParse.left);
    POINTERS_EQUAL(nullptr, gParse.right);
}

TEST(ExpressionParser, SimplePrecedence)
{
    //   +
    //  / \
    // 1   *
    //    / \
    //   2   3
    TOKENIZE("1 + 2 * 3");
    CHECK_SUCCESS(ExpressionParser::parse(it, gParse, nullptr));

    const ExpressionParser::Parse* node = nullptr;

    // 1 + ...
    node = gParse.left;
    CHECK_TRUE(node->data == toks[0]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);

    node = &gParse;
    CHECK_TRUE(node->data == toks[1]);

    // 2 * 3
    node = gParse.right->left;
    CHECK_TRUE(node->data == toks[2]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);

    node = gParse.right;
    CHECK_TRUE(node->data == toks[3]);

    node = gParse.right->right;
    CHECK_TRUE(node->data == toks[4]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);
}

TEST(ExpressionParser, SimplePrecedenceWithParens)
{
    //     *
    //    / \
    //   +   3
    //  / \
    // 1   2
    TOKENIZE("(1 + 2) * 3");
    CHECK_SUCCESS(ExpressionParser::parse(it, gParse, nullptr));

    const ExpressionParser::Parse* node = nullptr;

    // 1 + 2
    node = gParse.left->left;
    CHECK_TRUE(node->data == toks[1]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);

    node = gParse.left;
    CHECK_TRUE(node->data == toks[2]);

    node = gParse.left->right;
    CHECK_TRUE(node->data == toks[3]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);

    // ... * 3
    node = &gParse;
    CHECK_TRUE(node->data == toks[5]);

    node = gParse.right;
    CHECK_TRUE(node->data == toks[6]);
    POINTERS_EQUAL(nullptr, node->left);
    POINTERS_EQUAL(nullptr, node->right);
}
