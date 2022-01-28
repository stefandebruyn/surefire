#include <sstream>
#include <algorithm>

#include "sfa/config/ConfigTokenizer.hpp"
#include "UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define CHECK_TOKENS(kSrc, kToksExpect)                                        \
{                                                                              \
    std::stringstream ss(kSrc);                                                \
    std::vector<Token> toksActual;                                             \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toksActual, nullptr));         \
    CHECK_EQUAL(kToksExpect,  toksActual);                                     \
}

#define CHECK_TOKEN(kSrc, kType, kStr, kLineNum, kColNum)                      \
{                                                                              \
    const std::vector<Token> toksExpect =                                      \
    {                                                                          \
        {kType, kStr, kLineNum, kColNum}                                       \
    };                                                                         \
    CHECK_TOKENS(kSrc, toksExpect);                                            \
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(ConfigTokenizer)
{
};

TEST(ConfigTokenizer, TokenEquivalence)
{
    const Token a = {Token::CONSTANT, "foo", 0, 0};
    const Token b = {Token::CONSTANT, "foo", 0, 0};
    const Token c = {Token::LABEL,    "foo", 0, 0};
    const Token d = {Token::CONSTANT, "bar", 0, 0};
    const Token e = {Token::CONSTANT, "foo", 1, 0};
    const Token f = {Token::CONSTANT, "foo", 0, 1};
    CHECK_TRUE(a == b);
    CHECK_TRUE(a != c);
    CHECK_TRUE(a != d);
    CHECK_TRUE(a != e);
    CHECK_TRUE(a != f);
}

TEST(ConfigTokenizer, Section)
{
    CHECK_TOKEN("[Foo123_/]", Token::SECTION, "[Foo123_/]", 1, 1);
}

TEST(ConfigTokenizer, Label)
{
    CHECK_TOKEN(".Foo123_-][", Token::LABEL, ".Foo123_-][", 1, 1);
}

TEST(ConfigTokenizer, Identifier)
{
    CHECK_TOKEN("Foo123_", Token::IDENTIFIER, "Foo123_", 1, 1);
}

TEST(ConfigTokenizer, Operator)
{
    CHECK_TOKEN("==", Token::OPERATOR, "==", 1, 1);
    CHECK_TOKEN("!=", Token::OPERATOR, "!=", 1, 1);
    CHECK_TOKEN("=", Token::OPERATOR, "=", 1, 1);
    CHECK_TOKEN("<=", Token::OPERATOR, "<=", 1, 1);
    CHECK_TOKEN("<", Token::OPERATOR, "<", 1, 1);
    CHECK_TOKEN(">=", Token::OPERATOR, ">=", 1, 1);
    CHECK_TOKEN(">", Token::OPERATOR, ">", 1, 1);
    CHECK_TOKEN("->", Token::OPERATOR, "->", 1, 1);
    CHECK_TOKEN("and", Token::OPERATOR, "and", 1, 1);
    CHECK_TOKEN("or", Token::OPERATOR, "or", 1, 1);
    CHECK_TOKEN("+", Token::OPERATOR, "+", 1, 1);
    CHECK_TOKEN("-", Token::OPERATOR, "-", 1, 1);
    CHECK_TOKEN("*", Token::OPERATOR, "*", 1, 1);
    CHECK_TOKEN("/", Token::OPERATOR, "/", 1, 1);
}

TEST(ConfigTokenizer, Constant)
{
    CHECK_TOKEN("123", Token::CONSTANT, "123", 1, 1);
    CHECK_TOKEN(".123", Token::CONSTANT, ".123", 1, 1);
    CHECK_TOKEN("123.456", Token::CONSTANT, "123.456", 1, 1);
    CHECK_TOKEN("true", Token::CONSTANT, "true", 1, 1);
    CHECK_TOKEN("false", Token::CONSTANT, "false", 1, 1);
}

TEST(ConfigTokenizer, Colon)
{
    CHECK_TOKEN(":", Token::COLON, ":", 1, 1);
}

TEST(ConfigTokenizer, Newline)
{
    CHECK_TOKEN("\n", Token::NEWLINE, "(newline)", 1, 1);
}

TEST(ConfigTokenizer, LeftParen)
{
    CHECK_TOKEN("(", Token::LPAREN, "(", 1, 1);
}

TEST(ConfigTokenizer, RightParen)
{
    CHECK_TOKEN(")", Token::RPAREN, ")", 1, 1);
}

TEST(ConfigTokenizer, Annotation)
{
    CHECK_TOKEN("@Foo123_", Token::ANNOTATION, "@Foo123_", 1, 1);
}

TEST(ConfigTokenizer, Comment)
{
    const std::vector<Token> empty;
    CHECK_TOKENS("# hello world !#$%^", empty);
}

TEST(ConfigTokenizer, LeftBrace)
{
    CHECK_TOKEN("{", Token::LBRACE, "{", 1, 1);
}

TEST(ConfigTokenizer, RightBrace)
{
    CHECK_TOKEN("}", Token::RBRACE, "}", 1, 1);
}

TEST(ConfigTokenizer, EveryToken)
{
    const std::vector<Token> toksExpect =
    {
        {Token::NEWLINE, "(newline)", 1, 1},
        {Token::NEWLINE, "(newline)", 2, 1},
        {Token::RBRACE, "}", 3, 2},
        {Token::CONSTANT, ".3", 3, 7},
        {Token::IDENTIFIER, "foo", 3, 11},
        {Token::NEWLINE, "(newline)", 3, 14},
        {Token::NEWLINE, "(newline)", 4, 1},
        {Token::NEWLINE, "(newline)", 5, 6},
        {Token::SECTION, "[foo]", 6, 5},
        {Token::LPAREN, "(", 6, 10},
        {Token::LBRACE, "{", 6, 11},
        {Token::ANNOTATION, "@foo", 6, 12},
        {Token::NEWLINE, "(newline)", 6, 16},
        {Token::LABEL, ".foo", 7, 2},
        {Token::OPERATOR, "!=", 7, 9},
        {Token::CONSTANT, "false", 7, 11},
        {Token::NEWLINE, "(newline)", 7, 22},
        {Token::RPAREN, ")", 8, 1},
        {Token::COLON, ":", 8, 2},
        {Token::OPERATOR, "and", 8, 4},
        {Token::CONSTANT, "123", 8, 8}
    };
    CHECK_TOKENS(
        "\n"
        "\n"
        "\t}    .3  foo\n"
        "\n"
        "# foo\n"
        "    [foo]({@foo\n"
        " .foo   !=false # foo\n"
        "): and 123",
        toksExpect);
}

TEST(ConfigTokenizer, InvalidToken)
{
    // Tokenizer returns error on invalid token.
    std::stringstream ss("\n@foo 123.456\n foo! [foo]");
    std::vector<Token> toks;
    ConfigErrorInfo err;
    CHECK_EQUAL(E_TOK_INVALID, ConfigTokenizer::tokenize(ss, toks, &err));

    // Line and column numbers of offending substring are correctly identified.
    CHECK_EQUAL(3, err.lineNum);
    CHECK_EQUAL(5, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.msg.size() > 0);

    // Tokens vector was not populated.
    CHECK_EQUAL(0, toks.size());
}
