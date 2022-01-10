#include <sstream>

#include "sfa/ConfigTokenizer.hpp"
#include "utest/UTest.hpp"

#include <iostream> // rm later

TEST_GROUP(ConfigTokenizer)
{
};

TEST(ConfigTokenizer, TokenEquivalence)
{
    Token a = {TOK_CONSTANT, 0, "foo"};
    Token b = {TOK_CONSTANT, 0, "foo"};
    Token c = {TOK_LABEL,    0, "foo"};
    Token d = {TOK_CONSTANT, 1, "foo"};
    Token e = {TOK_CONSTANT, 0, "bar"};
    CHECK_TRUE(a == b);
    CHECK_TRUE(a != c);
    CHECK_TRUE(a != d);
    CHECK_TRUE(a != e);
}

TEST(ConfigTokenizer, SectionToken)
{
    std::stringstream ss("[Foo]");
    std::vector<Token> toksActual;
    CHECK_SUCCESS(Tokenizer::tokenize(ss, toksActual, nullptr));
    std::vector<Token> toksExpect =
    {
        {TOK_SECTION, 0, "Foo", 0, 0}
    };
    CHECK_TRUE(toksExpect == toksActual);
}

TEST(ConfigTokenizer, CommentToken)
{
    std::stringstream ss("# hello world");
    std::vector<Token> toksActual;
    ConfigErrorInfo err;
    CHECK_SUCCESS(Tokenizer::tokenize(ss, toksActual, &err));
    std::vector<Token> toksExpect;
    CHECK_TRUE(toksExpect == toksActual);
}

TEST(ConfigTokenizer, AnnotationToken)
{
    std::stringstream ss("@FOO");
    std::vector<Token> toksActual;
    CHECK_SUCCESS(Tokenizer::tokenize(ss, toksActual, nullptr));
    std::vector<Token> toksExpect =
    {
        {TOK_ANNOTATION, 0, "@FOO", 0, 0}
    };
    CHECK_TRUE(toksExpect == toksActual);
}
