#include "sfa/sup/ExpressionParser.hpp"
#include "sfa/utest/UTest.hpp"

#define TOKENIZE(kSrc)                                                         \
    std::stringstream ss(kSrc);                                                \
    std::vector<Token> toks;                                                   \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toks, nullptr));               \
    TokenIterator it(toks.begin(), toks.end());

TEST_GROUP(ExpressionParser)
{
};

TEST(ExpressionParser, OneConstant)
{
    TOKENIZE("10");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[0]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

TEST(ExpressionParser, OneVariable)
{
    TOKENIZE("foo");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[0]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

TEST(ExpressionParser, SimplePrecedence)
{
    //   +
    //  / \
    // 1   *
    //    / \
    //   2   3
    TOKENIZE("1 + 2 * 3");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // 1 +
    node = parse->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse;
    CHECK_TRUE(node->data == toks[1]);

    // 2 * 3
    node = parse->right->left;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->right;
    CHECK_TRUE(node->data == toks[3]);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, SimplePrecedenceWithParens)
{
    //     *
    //    / \
    //   +   3
    //  / \
    // 1   2
    TOKENIZE("(1 + 2) * 3");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // 1 + 2
    node = parse->left->left;
    CHECK_TRUE(node->data == toks[1]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left;
    CHECK_TRUE(node->data == toks[2]);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // * 3
    node = parse;
    CHECK_TRUE(node->data == toks[5]);

    node = parse->right;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, EqualPrecedence)
{
    //       +
    //      / \
    //     +   bar
    //    / \
    //   +   2
    //  / \
    // 1   foo
    TOKENIZE("1 + foo + 2 + bar");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // 1 + foo
    node = parse->left->left->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left;
    CHECK_TRUE(node->data == toks[1]);

    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // + 2
    node = parse->left;
    CHECK_TRUE(node->data == toks[3]);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // + bar
    node = parse;
    CHECK_TRUE(node->data == toks[5]);

    node = parse->right;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, UnaryOperator)
{
    // NOT
    //   \
    //   foo
    TOKENIZE("NOT foo");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right;
    CHECK_TRUE(node->data == toks[1]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, UnaryAndBinaryOperator)
{
    //   AND
    //   /  \
    // foo  NOT
    //        \
    //        bar
    TOKENIZE("foo AND NOT bar");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo AND
    node = parse->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse;
    CHECK_TRUE(node->data == toks[1]);

    // NOT bar
    node = parse->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, ArithmeticOperators)
{
    //     -
    //    / \
    //   +   \
    //  / \   \
    // a   b   \
    //          /
    //         / \
    //        *   f
    //       / \
    //      c   d
    TOKENIZE("a + b - c * d / f");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // a + b
    node = parse->left->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left;
    CHECK_TRUE(node->data == toks[1]);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // -
    node = parse;
    CHECK_TRUE(node->data == toks[3]);

    // c * d
    node = parse->right->left->left;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->right->left;
    CHECK_TRUE(node->data == toks[5]);

    node = parse->right->left->right;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // / f
    node = parse->right;
    CHECK_TRUE(node->data == toks[7]);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[8]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, ComparisonOperators)
{
    //         ==
    //        / \
    //       !=  \
    //      / \   \
    //     ==  \   \
    //    / \   \   \
    //   <   \   \   \
    //  / \   \   \   \
    // a   b   \   \   \
    //          <=  \   \
    //         / \   \   \
    //        c   d   \   \
    //                 >   \
    //                / \   \
    //               e   f   \
    //                        >=
    //                       / \
    //                      g   h
    TOKENIZE("a < b == c <= d != e > f == g >= h");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // a < b
    node = parse->left->left->left->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left->left;
    CHECK_TRUE(node->data == toks[1]);

    node = parse->left->left->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // ==
    node = parse->left->left;
    CHECK_TRUE(node->data == toks[3]);

    // c <= d
    node = parse->left->left->right->left;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[5]);

    node = parse->left->left->right->right;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // !=
    node = parse->left;
    CHECK_TRUE(node->data == toks[7]);

    // e > f
    node = parse->left->right->left;
    CHECK_TRUE(node->data == toks[8]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[9]);

    node = parse->left->right->right;
    CHECK_TRUE(node->data == toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // ==
    node = parse;
    CHECK_TRUE(node->data == toks[11]);

    // g >= h
    node = parse->right->left;
    CHECK_TRUE(node->data == toks[12]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->right;
    CHECK_TRUE(node->data == toks[13]);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[14]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, LogicalOperators)
{
    //     OR
    //    / \
    //   AND \
    //  / \   \
    // a   b   \
    //          NOT
    //           \
    //            c
    TOKENIZE("a AND b OR NOT c");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // a AND b
    node = parse->left->left;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left;
    CHECK_TRUE(node->data == toks[1]);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // OR
    node = parse;
    CHECK_TRUE(node->data == toks[3]);

    // NOT c
    node = parse->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, NestedParentheses)
{
    //       AND
    //      / \
    //     OR  e
    //    /  \
    //   AND  d
    //  / \
    // a   OR
    //    / \
    //   b   c
    TOKENIZE("((a AND (b OR c)) OR d) AND e");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // a AND
    node = parse->left->left->left;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left;
    CHECK_TRUE(node->data == toks[3]);

    // b OR c
    node = parse->left->left->right->left;
    CHECK_TRUE(node->data == toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[6]);

    node = parse->left->left->right->right;
    CHECK_TRUE(node->data == toks[7]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // OR d
    node = parse->left;
    CHECK_TRUE(node->data == toks[10]);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[11]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // AND e
    node = parse;
    CHECK_TRUE(node->data == toks[13]);

    node = parse->right;
    CHECK_TRUE(node->data == toks[14]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, ExtraParenthesesOnOneTerm)
{
    TOKENIZE("(((a)))");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[3]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

TEST(ExpressionParser, ErrorNoTokens)
{
    TOKENIZE("");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_EMPTY, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorTooManyLeftParentheses)
{
    TOKENIZE("((a + b) * c");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_PAREN, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorTooManyRightParentheses)
{
    TOKENIZE("(a + b) * c)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_PAREN, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorUnknownOperator)
{
    TOKENIZE("a + b");
    toks[1].str = "foo";
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_OP, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorUnexpectedToken)
{
    TOKENIZE("a + b @foo");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_TOK, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorNoTermsInExpression)
{
    TOKENIZE("()");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_EMPTY, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorSyntaxMissingOperator)
{
    TOKENIZE("a b");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_SYNTAX, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorSyntaxBinaryOperatorMissingLhs)
{
    TOKENIZE("+ a");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_SYNTAX, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorSyntaxBinaryOperatorMissingRhs)
{
    TOKENIZE("a +");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_SYNTAX, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorSyntaxUnaryOperatorMissingRhs)
{
    TOKENIZE("a NOT");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_SYNTAX, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}

TEST(ExpressionParser, ErrorSyntaxAdjacentBinaryOperators)
{
    TOKENIZE("a + + b");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_SYNTAX, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse.get() == nullptr);
}
