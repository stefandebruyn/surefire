#include "sf/config/ExpressionParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define CHECK_ARG_CNT(kFuncNode, kExpectCnt)                                   \
{                                                                              \
    U32 cnt = 0;                                                               \
    std::shared_ptr<ExpressionParser::Parse> _node = kFuncNode;                \
    while (_node->left)                                                        \
    {                                                                          \
        ++cnt;                                                                 \
        _node = _node->left;                                                   \
    }                                                                          \
    CHECK_EQUAL(kExpectCnt, cnt);                                              \
}

static void checkParsesEqual(
    const std::shared_ptr<ExpressionParser::Parse> kNodeA,
    const std::shared_ptr<ExpressionParser::Parse> kNodeB)
{
    CHECK_EQUAL((kNodeA == nullptr), (kNodeB == nullptr));

    if (kNodeA == nullptr)
    {
        return;
    }

    CHECK_EQUAL(kNodeA->data.type, kNodeB->data.type);
    CHECK_EQUAL(kNodeA->data.str, kNodeB->data.str);

    checkParsesEqual(kNodeA->left, kNodeB->left);
    checkParsesEqual(kNodeA->right, kNodeB->right);
}

static void checkParseError(TokenIterator& kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code.
    ErrorInfo err;
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(kRes, ExpressionParser::parse(kIt, parse, &err));

    // Line and column numbers of offending token are correctly identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);
}

///////////////////////////////// Usage Tests //////////////////////////////////

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

TEST(ExpressionParser, BinaryOperatorLeftAssociativity)
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

TEST(ExpressionParser, UnaryOperatorRightAssociativity)
{
    // NOT
    //  \
    //   NOT
    //    \
    //     a
    TOKENIZE("NOT NOT a");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // NOT
    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    // NOT a
    node = parse->right;
    CHECK_TRUE(node->data == toks[1]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, ParenthesesAfterBinaryOperator)
{
    //   +
    //  / \
    // 1   +
    //    / \
    //   2   3
    TOKENIZE("1 + (2 + 3)");
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

    // 2 + 3
    node = parse->right->left;
    CHECK_TRUE(node->data == toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->right;
    CHECK_TRUE(node->data == toks[4]);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, ParenthesesAfterUnaryOperator)
{
    // NOT
    //  \
    //   AND
    //  / \
    // a   b
    TOKENIZE("NOT (a AND b)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // NOT
    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    // a AND b
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

TEST(ExpressionParser, ExpandDoubleInequalityLtLte)
{
    TOKENIZE("a < b <= c");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> parseExpect;
    {
        TOKENIZE("a < b AND b <= c");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

TEST(ExpressionParser, ExpandDoubleInequalityGtGte)
{
    TOKENIZE("a > b >= c");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> parseExpect;
    {
        TOKENIZE("a > b AND b >= c");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

TEST(ExpressionParser, ExpandTripleInequality)
{
    TOKENIZE("a < b < c < d");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> parseExpect;
    {
        TOKENIZE("a < b AND b < c AND c < d");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

TEST(ExpressionParser, ExpandDoubleInequalityNestedExpression)
{
    TOKENIZE("a + b < c + d < e + f");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> parseExpect;
    {
        TOKENIZE("a + b < c + d AND c + d < e + f");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

TEST(ExpressionParser, FunctionCallNoArgs)
{
    TOKENIZE("foo()");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 0);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);
}

TEST(ExpressionParser, FunctionCallOneArg)
{
    //   foo
    //  /
    // arg1
    //  \
    //   a
    TOKENIZE("foo(a)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a
    node = parse->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallTwoArgs)
{
    //      foo
    //     /
    //    arg1
    //   / \
    //  /   a
    // arg2
    //  \
    //   b
    TOKENIZE("foo(a, b)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 2);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a
    node = parse->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b
    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallThreeArgs)
{
    //         foo
    //        /
    //       arg1
    //      / \
    //     /   a
    //    arg2
    //   / \
    //  /   b
    // arg3
    //  \
    //   c
    TOKENIZE("foo(a, b, c)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 3);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a
    node = parse->left->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b
    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // c
    node = parse->left->left->left->right;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallExpressionArg)
{
    //   foo
    //  /
    // arg1
    //  \
    //   +
    //  / \
    // a   b
    TOKENIZE("foo(a + b)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a + b
    node = parse->left->right->left;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[3]);

    node = parse->left->right->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallTwoExpressionArgs)
{
    //        foo
    //       /
    //      arg1
    //     / \
    //    /   +
    //   /   / \
    //  /   a   b
    // arg2
    //  \
    //   OR
    //  / \
    // c   d
    TOKENIZE("foo(a + b, c OR d)");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 2);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a + b
    node = parse->left->right->left;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[3]);

    node = parse->left->right->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // c OR d
    node = parse->left->left->right->left;
    CHECK_TRUE(node->data == toks[6]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[7]);

    node = parse->left->left->right->right;
    CHECK_TRUE(node->data == toks[8]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallParenthesizedExpressionArg)
{
    //   foo
    //  /
    // arg1
    //  \
    //   +
    //  / \
    // a   b
    TOKENIZE("foo((a + b))");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a + b
    node = parse->left->right->left;
    CHECK_TRUE(node->data == toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[4]);

    node = parse->left->right->right;
    CHECK_TRUE(node->data == toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, FunctionCallMultipleParenthesizedExpressionArgs)
{
    //        foo
    //       /
    //      arg1
    //     / \
    //    /   +
    //   /   / \
    //  /   a   b
    // arg2
    //  \
    //   OR
    //  / \
    // c   d
    TOKENIZE("foo((a + b), (c OR d))");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 2);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a + b
    node = parse->left->right->left;
    CHECK_TRUE(node->data == toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->right;
    CHECK_TRUE(node->data == toks[4]);

    node = parse->left->right->right;
    CHECK_TRUE(node->data == toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // c OR d
    node = parse->left->left->right->left;
    CHECK_TRUE(node->data == toks[9]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = parse->left->left->right;
    CHECK_TRUE(node->data == toks[10]);

    node = parse->left->left->right->right;
    CHECK_TRUE(node->data == toks[11]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, NestedFunctionCall)
{
    //   foo
    //  /
    // foo-arg1
    //  \
    //   bar
    //  /
    // bar-arg1
    //  \
    //   a
    TOKENIZE("foo(bar(a))");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // bar
    node = parse->left->right;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a
    node = parse->left->right->left->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(ExpressionParser, MultipleNestedFunctionCalls)
{
    //           foo
    //          /
    //         foo-arg1
    //        / \
    //       /   bar
    //      /   /
    //     /   bar-arg1
    //    /     \
    //   /       a
    //  foo-arg2
    //   \
    //    baz
    //   /
    //  baz-arg1
    //   \
    //    b
    TOKENIZE("foo(bar(a), baz(b))");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    std::shared_ptr<ExpressionParser::Parse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 2);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // bar
    node = parse->left->right;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // a
    node = parse->left->right->left->right;
    CHECK_TRUE(node->data == toks[4]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // baz
    node = parse->left->left->right;
    CHECK_ARG_CNT(node, 1);
    CHECK_TRUE(node->data == toks[7]);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);

    // b
    node = parse->left->left->right->left->right;
    CHECK_TRUE(node->data == toks[9]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(ExpressionParserErrors)
{
};

TEST(ExpressionParserErrors, FunctionCallLoneComma)
{
    TOKENIZE("foo(,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

TEST(ExpressionParserErrors, FunctionCallTrailingComma)
{
    TOKENIZE("foo(a,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 7);
}

TEST(ExpressionParserErrors, FunctionCallLeadingComma)
{
    TOKENIZE("foo(,a)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

TEST(ExpressionParserErrors, FunctionCallSequentialCommas)
{
    TOKENIZE("foo(,,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

TEST(ExpressionParserErrors, SyntaxErrorInFunctionCallArgument)
{
    TOKENIZE("foo(a +)");
    checkParseError(it, E_EXP_SYNTAX, 1, 7);
}

TEST(ExpressionParserErrors, NoTokens)
{
    TOKENIZE("");
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_EMPTY, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse == nullptr);
}

TEST(ExpressionParserErrors, TooManyLeftParentheses)
{
    TOKENIZE("((a + b) * c");
    checkParseError(it, E_EXP_PAREN, 1, 1);
}

TEST(ExpressionParserErrors, TooManyRightParentheses)
{
    TOKENIZE("(a + b) * c)");
    checkParseError(it, E_EXP_PAREN, 1, 12);
}

TEST(ExpressionParserErrors, UnknownOperator)
{
    TOKENIZE("a + b");
    toks[1].str = "foo";
    std::shared_ptr<ExpressionParser::Parse> parse;
    CHECK_ERROR(E_EXP_OP, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse == nullptr);
}

TEST(ExpressionParserErrors, UnexpectedToken)
{
    TOKENIZE("a + b @foo");
    checkParseError(it, E_EXP_TOK, 1, 7);
}

TEST(ExpressionParserErrors, NoTermsInExpression)
{
    TOKENIZE("()");
    checkParseError(it, E_EXP_EMPTY, 1, 1);
}

TEST(ExpressionParserErrors, SyntaxMissingOperator)
{
    TOKENIZE("a b");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

TEST(ExpressionParserErrors, SyntaxBinaryOperatorMissingLhs)
{
    TOKENIZE("+ a");
    checkParseError(it, E_EXP_SYNTAX, 1, 1);
}

TEST(ExpressionParserErrors, SyntaxBinaryOperatorMissingRhs)
{
    TOKENIZE("a +");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

TEST(ExpressionParserErrors, SyntaxUnaryOperatorMissingRhs)
{
    TOKENIZE("a NOT");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

TEST(ExpressionParserErrors, SyntaxAdjacentBinaryOperators)
{
    TOKENIZE("a + + b");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}
