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
/// @file  sf/core/utest/UTestTokenizer.cpp
/// @brief Unit tests for Tokenizer.
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <sstream>

#include "sf/config/Tokenizer.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that a string is tokenized in the expected way.
///
/// @param[in] kSrc         String to tokenize.
/// @param[in] kToksExpect  Expected vector of tokens.
///
#define CHECK_TOKENS(kSrc, kToksExpect)                                        \
{                                                                              \
    std::stringstream ss(kSrc);                                                \
    Vec<Token> toksActual;                                                     \
    CHECK_SUCCESS(Tokenizer::tokenize(ss, toksActual, nullptr));               \
    CHECK_EQUAL(kToksExpect,  toksActual);                                     \
}

///
/// @brief Checks that a string tokenizes into a single token.
///
/// @param[in] kSrc      String to tokenize.
/// @param[in] kType     Expected token type.
/// @param[in] kStr      Expected token string.
/// @param[in] kLineNum  Expected token line number.
/// @param[in] kColNum   Expected token column number.
///
#define CHECK_TOKEN(kSrc, kType, kStr, kLineNum, kColNum)                      \
{                                                                              \
    const Vec<Token> toksExpect =                                              \
    {                                                                          \
        {kType, kStr, kLineNum, kColNum}                                       \
    };                                                                         \
    CHECK_TOKENS(kSrc, toksExpect);                                            \
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for Tokenizer.
///
TEST_GROUP(Tokenizer)
{
};

///
/// @test Tokens are equated correctly.
///
TEST(Tokenizer, TokenEquivalence)
{
    const Token a = {Token::CONSTANT, "foo", 0, 0};
    const Token b = {Token::CONSTANT, "foo", 0, 0};
    const Token c = {Token::LABEL,    "foo", 0, 0};
    const Token d = {Token::CONSTANT, "bar", 0, 0};
    const Token e = {Token::CONSTANT, "foo", 1, 0};
    const Token f = {Token::CONSTANT, "foo", 0, 1};

    CHECK_TRUE(a == b);
    CHECK_TRUE(b == a);

    CHECK_TRUE(a != c);
    CHECK_TRUE(a != d);
    CHECK_TRUE(a != e);
    CHECK_TRUE(a != f);
    CHECK_TRUE(c != a);
    CHECK_TRUE(d != a);
    CHECK_TRUE(e != a);
    CHECK_TRUE(f != a);
}

///
/// @test A section is tokenized correctly.
///
TEST(Tokenizer, Section)
{
    CHECK_TOKEN("[Foo123_/]", Token::SECTION, "[Foo123_/]", 1, 1);
}

///
/// @test A label is tokenized correctly.
///
TEST(Tokenizer, Label)
{
    CHECK_TOKEN(".Foo123_-][", Token::LABEL, ".Foo123_-][", 1, 1);
}

///
/// @test An identifier is tokenized correctly.
///
TEST(Tokenizer, Identifier)
{
    CHECK_TOKEN("Foo123_", Token::IDENTIFIER, "Foo123_", 1, 1);
}

///
/// @test Operators are tokenized correctly.
///
TEST(Tokenizer, Operator)
{
    CHECK_TOKEN("==", Token::OPERATOR, "==", 1, 1);
    CHECK_TOKEN("!=", Token::OPERATOR, "!=", 1, 1);
    CHECK_TOKEN("=", Token::OPERATOR, "=", 1, 1);
    CHECK_TOKEN("<=", Token::OPERATOR, "<=", 1, 1);
    CHECK_TOKEN("<", Token::OPERATOR, "<", 1, 1);
    CHECK_TOKEN(">=", Token::OPERATOR, ">=", 1, 1);
    CHECK_TOKEN(">", Token::OPERATOR, ">", 1, 1);
    CHECK_TOKEN("and", Token::OPERATOR, "and", 1, 1);
    CHECK_TOKEN("or", Token::OPERATOR, "or", 1, 1);
    CHECK_TOKEN("+", Token::OPERATOR, "+", 1, 1);
    CHECK_TOKEN("-", Token::OPERATOR, "-", 1, 1);
    CHECK_TOKEN("*", Token::OPERATOR, "*", 1, 1);
    CHECK_TOKEN("/", Token::OPERATOR, "/", 1, 1);
}

///
/// @test Constants are tokenized correctly.
///
TEST(Tokenizer, Constant)
{
    CHECK_TOKEN("123", Token::CONSTANT, "123", 1, 1);
    CHECK_TOKEN(".123", Token::CONSTANT, ".123", 1, 1);
    CHECK_TOKEN("123.456", Token::CONSTANT, "123.456", 1, 1);
    CHECK_TOKEN("true", Token::CONSTANT, "true", 1, 1);
    CHECK_TOKEN("false", Token::CONSTANT, "false", 1, 1);
}

///
/// @test A colon is tokenized correctly.
///
TEST(Tokenizer, Colon)
{
    CHECK_TOKEN(":", Token::COLON, ":", 1, 1);
}

///
/// @test A newline is tokenized correctly.
///
TEST(Tokenizer, Newline)
{
    CHECK_TOKEN("\n", Token::NEWLINE, "(newline)", 1, 1);
}

///
/// @test A left parenthese is tokenized correctly.
///
TEST(Tokenizer, LeftParen)
{
    CHECK_TOKEN("(", Token::LPAREN, "(", 1, 1);
}

///
/// @test A right parenthese is tokenized correctly.
///
TEST(Tokenizer, RightParen)
{
    CHECK_TOKEN(")", Token::RPAREN, ")", 1, 1);
}

///
/// @test An annotation is tokenized correctly.
///
TEST(Tokenizer, Annotation)
{
    CHECK_TOKEN("@Foo123_", Token::ANNOTATION, "@Foo123_", 1, 1);
}

///
/// @test Comments are ignored by the tokenizer.
///
TEST(Tokenizer, Comment)
{
    const Vec<Token> empty;
    CHECK_TOKENS("# hello world !#$%^", empty);
}

///
/// @test A left brace is tokenized correctly.
///
TEST(Tokenizer, LeftBrace)
{
    CHECK_TOKEN("{", Token::LBRACE, "{", 1, 1);
}

///
/// @test A right brace is tokenized correctly.
///
TEST(Tokenizer, RightBrace)
{
    CHECK_TOKEN("}", Token::RBRACE, "}", 1, 1);
}

///
/// @test A comma brace is tokenized correctly.
///
TEST(Tokenizer, Comma)
{
    CHECK_TOKEN(",", Token::COMMA, ",", 1, 1);
}

///
/// @test Keywords are tokenized correctly.
///
TEST(Tokenizer, Keyword)
{
    CHECK_TOKEN("if", Token::KEYWORD, "if", 1, 1);
    CHECK_TOKEN("else", Token::KEYWORD, "else", 1, 1);
    CHECK_TOKEN("->", Token::KEYWORD, "->", 1, 1);
}

///
/// @test A string of only spaces generates no tokens.
///
TEST(Tokenizer, OnlySpaces)
{
    const Vec<Token> empty;
    CHECK_TOKENS("    ", empty);
}

///
/// @test A string of spaces and then a newline is tokenized correctly.
///
TEST(Tokenizer, OnlySpacesThenNewline)
{
    CHECK_TOKEN("    \n", Token::NEWLINE, "(newline)", 1, 5);
}

///
/// @test A string containing all token types is tokenized correctly.
///
TEST(Tokenizer, EveryToken)
{
    const Vec<Token> toksExpect =
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
        {Token::CONSTANT, "123", 8, 8},
        {Token::COMMA, ",", 8, 11},
        {Token::COMMA, ",", 8, 12},
        {Token::KEYWORD, "->", 8, 13},
    };
    CHECK_TOKENS(
        "\n"
        "\n"
        "\t}    .3  foo\n"
        "\n"
        "# foo\n"
        "    [foo]({@foo\n"
        " .foo   !=false # foo\n"
        "): and 123,,->",
        toksExpect);
}

///
/// @test An invalid token generates an error.
///
TEST(Tokenizer, InvalidToken)
{
    // Tokenizer returns error on invalid token.
    std::stringstream ss(
        "\n"
        "@foo 123.456\n"
        " foo$ [foo]"); // `$` is invalid
    Vec<Token> toks;
    ErrorInfo err;
    CHECK_ERROR(E_TOK_INVALID, Tokenizer::tokenize(ss, toks, &err));

    // Line and column numbers of offending substring are correctly identified.
    CHECK_EQUAL(3, err.lineNum);
    CHECK_EQUAL(5, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Tokens vector was not populated.
    CHECK_EQUAL(0, toks.size());
}

///
/// @test A nonexistent input file generates an error.
///
TEST(Tokenizer, NonexistentInputFile)
{
    // Tokenizer returns error on nonexistent input file.
    Vec<Token> toks;
    ErrorInfo err;
    CHECK_ERROR(E_TOK_FILE, Tokenizer::tokenize("foo.bar", toks, &err));

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}
