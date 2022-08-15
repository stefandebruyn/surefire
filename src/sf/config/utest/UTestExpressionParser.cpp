////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/utest/UTestExpressionParser.hpp
/// @brief Unit tests for ExpressionParser.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/ExpressionParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that a function parse has the expected argument count.
///
/// @param[in] kFuncNode   Function parse node, of type
///                        Ref<const ExpressionParse>.
/// @param[in] kExpectCnt  Expected number of arguments.
///
#define CHECK_ARG_CNT(kFuncNode, kExpectCnt)                                   \
{                                                                              \
    U32 cnt = 0;                                                               \
    Ref<const ExpressionParse> _node = kFuncNode;                              \
    while (_node->left)                                                        \
    {                                                                          \
        ++cnt;                                                                 \
        _node = _node->left;                                                   \
    }                                                                          \
    CHECK_EQUAL(kExpectCnt, cnt);                                              \
}

///
/// @brief Recursively checks if two expression parses contain the same tree
/// structure, with the same tokens at each node.
///
/// @param[in] kNodeA  An expression parse.
/// @param[in] kNodeB  Another expression parse.
///
static void checkParsesEqual(const Ref<const ExpressionParse> kNodeA,
                             const Ref<const ExpressionParse> kNodeB)
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

///
/// @brief Checks that parsing an expression generates the expected error.
///
/// @param[in] kIt       Iterator of tokens to parse.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error line number.
/// @param[in] kColNum   Expected error column number.
///
static void checkParseError(TokenIterator& kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code.
    ErrorInfo err;
    Ref<const ExpressionParse> parse;
    CHECK_ERROR(kRes, ExpressionParser::parse(kIt, parse, &err));

    // Line and column numbers of offending token are correctly identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, ExpressionParser::parse(kIt, parse, nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for ExpressionParser.
///
TEST_GROUP(ExpressionParser)
{
};

///
/// @test An expression with a single constant is parsed correctly.
///
TEST(ExpressionParser, OneConstant)
{
    TOKENIZE("10");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[0]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

///
/// @test An expression with a single variable is parsed correctly.
///
TEST(ExpressionParser, OneVariable)
{
    TOKENIZE("foo");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[0]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

///
/// @test A simple expression with operators of differing precedence is parsed
/// correctly.
///
TEST(ExpressionParser, SimplePrecedence)
{
    //   +
    //  / \
    // 1   *
    //    / \
    //   2   3
    TOKENIZE("1 + 2 * 3");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A simple expression with parentheses and operators of differing
/// precedence is parsed correctly.
///
TEST(ExpressionParser, SimplePrecedenceWithParens)
{
    //     *
    //    / \
    //   +   3
    //  / \
    // 1   2
    TOKENIZE("(1 + 2) * 3");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Binary operators are left-associative when equal precedence.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A unary operator is parsed correctly.
///
TEST(ExpressionParser, UnaryOperator)
{
    // not
    //   \
    //   foo
    TOKENIZE("not foo");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right;
    CHECK_TRUE(node->data == toks[1]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

///
/// @test Unary and binary operators together are parsed correctly.
///
TEST(ExpressionParser, UnaryAndBinaryOperator)
{
    //   and
    //  /  \
    // foo  not
    //       \
    //        bar
    TOKENIZE("foo and not bar");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test All arithmetic operators are parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test All comparison operators are parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test All logical operators are parsed correctly.
///
TEST(ExpressionParser, LogicalOperators)
{
    //     or
    //    / \
    //   and \
    //  / \   \
    // a   b   \
    //          not
    //           \
    //            c
    TOKENIZE("a and b or not c");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Nested parenthese are parsed correctly.
///
TEST(ExpressionParser, NestedParentheses)
{
    //       and
    //      / \
    //     or  e
    //    /  \
    //   and  d
    //  / \
    // a   or
    //    / \
    //   b   c
    TOKENIZE("((a and (b or c)) or d) and e");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Extraneous nested parentheses are parsed correctly.
///
TEST(ExpressionParser, ExtraParenthesesOnOneTerm)
{
    TOKENIZE("(((a)))");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse->data == toks[3]);
    CHECK_TRUE(parse->left == nullptr);
    CHECK_TRUE(parse->right == nullptr);
}

///
/// @test Unary operators are right-associative when equal precedence.
///
TEST(ExpressionParser, UnaryOperatorRightAssociativity)
{
    // not
    //  \
    //   not
    //    \
    //     a
    TOKENIZE("not not a");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

    // not
    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    // not a
    node = parse->right;
    CHECK_TRUE(node->data == toks[1]);
    CHECK_TRUE(node->left == nullptr);

    node = parse->right->right;
    CHECK_TRUE(node->data == toks[2]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

///
/// @test Parentheses are a binary operator are parsed correctly.
///
TEST(ExpressionParser, ParenthesesAfterBinaryOperator)
{
    //   +
    //  / \
    // 1   +
    //    / \
    //   2   3
    TOKENIZE("1 + (2 + 3)");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Parentheses are a unary operator are parsed correctly.
///
TEST(ExpressionParser, ParenthesesAfterUnaryOperator)
{
    // not
    //  \
    //   and
    //  / \
    // a   b
    TOKENIZE("not (a and b)");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

    // not
    node = parse;
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);

    // a and b
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

///
/// @test A double inequality with < and <= is expanded correctly.
///
TEST(ExpressionParser, ExpandDoubleInequalityLtLte)
{
    TOKENIZE("a < b <= c");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> parseExpect;
    {
        TOKENIZE("a < b and b <= c");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

///
/// @test A double inequality with > and >= is expanded correctly.
///
TEST(ExpressionParser, ExpandDoubleInequalityGtGte)
{
    TOKENIZE("a > b >= c");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> parseExpect;
    {
        TOKENIZE("a > b and b >= c");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

///
/// @test A triple inequality is expanded correctly.
///
TEST(ExpressionParser, ExpandTripleInequality)
{
    TOKENIZE("a < b < c < d");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> parseExpect;
    {
        TOKENIZE("a < b and b < c and c < d");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

///
/// @test A double inequality with more than single terms between the operators
/// is expanded correctly.
///
TEST(ExpressionParser, ExpandDoubleInequalityNestedExpression)
{
    TOKENIZE("a + b < c + d < e + f");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> parseExpect;
    {
        TOKENIZE("a + b < c + d and c + d < e + f");
        CHECK_SUCCESS(ExpressionParser::parse(it, parseExpect, nullptr));
    }

    checkParsesEqual(parseExpect, parse);
}

///
/// @test A function call with no arguments is parsed correctly.
///
TEST(ExpressionParser, FunctionCallNoArgs)
{
    TOKENIZE("foo()");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

    // foo
    node = parse;
    CHECK_ARG_CNT(node, 0);
    CHECK_TRUE(node->data == toks[0]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(node->func);
}

///
/// @test A function call with one argument is parsed correctly.
///
TEST(ExpressionParser, FunctionCallOneArg)
{
    //   foo
    //  /
    // arg1
    //  \
    //   a
    TOKENIZE("foo(a)");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with two arguments is parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with three arguments is parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with an argument that is more than a single term is
/// parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with two argumens that are more than a single term is
/// parsed correctly.
///
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
    //   or
    //  / \
    // c   d
    TOKENIZE("foo(a + b, c or d)");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with a parenthesized argument is parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test A function call with multiple parenthesized arguments is parsed
/// correctly.
///
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
    //   or
    //  / \
    // c   d
    TOKENIZE("foo((a + b), (c or d))");
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Nested function calls are parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @test Multiple nested function calls are parsed correctly.
///
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
    Ref<const ExpressionParse> parse;
    CHECK_SUCCESS(ExpressionParser::parse(it, parse, nullptr));

    Ref<const ExpressionParse> node;

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

///
/// @brief Unit tests for ExpressionParser errors.
///
TEST_GROUP(ExpressionParserErrors)
{
};

///
/// @test A function call with just a comma generates an error.
///
TEST(ExpressionParserErrors, FunctionCallLoneComma)
{
    TOKENIZE("foo(,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

///
/// @test A function call with a trailing comma generates an error.
///
TEST(ExpressionParserErrors, FunctionCallTrailingComma)
{
    TOKENIZE("foo(a,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 7);
}

///
/// @test A function call with a leading comma generates an error.
///
TEST(ExpressionParserErrors, FunctionCallLeadingComma)
{
    TOKENIZE("foo(,a)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

///
/// @test A function call with multiple sequential commas generates an error.
///
TEST(ExpressionParserErrors, FunctionCallSequentialCommas)
{
    TOKENIZE("foo(,,)");
    checkParseError(it, E_EXP_SYNTAX, 1, 5);
}

///
/// @test A function call containing an invalid expression generates an error.
///
TEST(ExpressionParserErrors, SyntaxErrorInFunctionCallArgument)
{
    TOKENIZE("foo(a +)");
    checkParseError(it, E_EXP_SYNTAX, 1, 7);
}

///
/// @test Parsing an empty expression generates an error.
///
TEST(ExpressionParserErrors, NoTokens)
{
    TOKENIZE("");
    Ref<const ExpressionParse> parse;
    CHECK_ERROR(E_EXP_EMPTY, ExpressionParser::parse(it, parse, nullptr));
    CHECK_TRUE(parse == nullptr);
}

///
/// @test Unbalanced opening parenthese generates an error.
///
TEST(ExpressionParserErrors, TooManyLeftParentheses)
{
    TOKENIZE("((a + b) * c");
    checkParseError(it, E_EXP_PAREN, 1, 1);
}

///
/// @test Unbalanced closing parenthese generates an error.
///
TEST(ExpressionParserErrors, TooManyRightParentheses)
{
    TOKENIZE("(a + b) * c)");
    checkParseError(it, E_EXP_PAREN, 1, 12);
}

///
/// @test Unexpected token in expression generates an error.
///
TEST(ExpressionParserErrors, UnexpectedToken)
{
    TOKENIZE("a + b @foo");
    checkParseError(it, E_EXP_TOK, 1, 7);
}

///
/// @test Parsing an expression of only parentheses generates an error.
///
TEST(ExpressionParserErrors, NoTermsInExpression)
{
    TOKENIZE("()");
    checkParseError(it, E_EXP_EMPTY, 1, 1);
}

///
/// @test Two sequential identifiers generates an error.
///
TEST(ExpressionParserErrors, SyntaxMissingOperator)
{
    TOKENIZE("a b");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

///
/// @test A binary operator with no left operand generates an error.
///
TEST(ExpressionParserErrors, SyntaxBinaryOperatorMissingLhs)
{
    TOKENIZE("+ a");
    checkParseError(it, E_EXP_SYNTAX, 1, 1);
}

///
/// @test A binary operator with no right operand generates an error.
///
TEST(ExpressionParserErrors, SyntaxBinaryOperatorMissingRhs)
{
    TOKENIZE("a +");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

///
/// @test A unary operator with no right operand generates an error.
///
TEST(ExpressionParserErrors, SyntaxUnaryOperatorMissingRhs)
{
    TOKENIZE("a NOT");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

///
/// @test Two sequential binary operators generates an error.
///
TEST(ExpressionParserErrors, SyntaxAdjacentBinaryOperators)
{
    TOKENIZE("a + + b");
    checkParseError(it, E_EXP_SYNTAX, 1, 3);
}

///
/// @test Parsing an expression with an assignment operator generates an error.
///
TEST(ExpressionParserErrors, IllegalAssignmentOperator)
{
    TOKENIZE("a = b");
    checkParseError(it, E_EXP_OP, 1, 3);
}
