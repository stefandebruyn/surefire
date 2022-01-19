#include <sstream>

#include "sfa/config/ConfigTokenizer.hpp"
#include "UTest.hpp"

TEST_GROUP(ConfigTokenizer)
{
};

TEST(ConfigTokenizer, TokenEquivalence)
{
    Token a = {TOK_CONSTANT, 0, "foo", 0, 0};
    Token b = {TOK_CONSTANT, 0, "foo", 0, 0};
    Token c = {TOK_LABEL,    0, "foo", 0, 0};
    Token d = {TOK_CONSTANT, 1, "foo", 0, 0};
    Token e = {TOK_CONSTANT, 0, "bar", 0, 0};
    Token f = {TOK_CONSTANT, 0, "foo", 1, 0};
    Token g = {TOK_CONSTANT, 0, "foo", 0, 1};
    CHECK_TRUE(a == b);
    CHECK_TRUE(a != c);
    CHECK_TRUE(a != d);
    CHECK_TRUE(a != e);
    CHECK_TRUE(a != f);
    CHECK_TRUE(a != g);
}

TEST(ConfigTokenizer, SectionToken)
{
    std::stringstream ss("[Foo]");
    std::vector<Token> toksActual;
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toksActual, nullptr));
    std::vector<Token> toksExpect =
    {
        {TOK_SECTION, 0, "Foo", 1, 1}
    };
    CHECK_TRUE(toksExpect == toksActual);
}

TEST(ConfigTokenizer, CommentToken)
{
    std::stringstream ss("# hello world");
    std::vector<Token> toksActual;
    ConfigErrorInfo err;
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toksActual, &err));
    std::vector<Token> toksExpect;
    CHECK_TRUE(toksExpect == toksActual);
}

TEST(ConfigTokenizer, AnnotationToken)
{
    std::stringstream ss("@FOO");
    std::vector<Token> toksActual;
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toksActual, nullptr));
    std::vector<Token> toksExpect =
    {
        {TOK_ANNOTATION, 0, "@FOO", 1, 1}
    };
    CHECK_TRUE(toksExpect == toksActual);
}
