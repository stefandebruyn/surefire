#include <sstream>

#include "sfa/config/ConfigTokenizer.hpp"
#include "UTest.hpp"

TEST_GROUP(ConfigTokenizer)
{
};

TEST(ConfigTokenizer, TokenEquivalence)
{
    Token a = {TOK_CONSTANT, "foo", 0, 0};
    Token b = {TOK_CONSTANT, "foo", 0, 0};
    Token c = {TOK_LABEL,    "foo", 0, 0};
    Token d = {TOK_CONSTANT, "bar", 0, 0};
    Token e = {TOK_CONSTANT, "foo", 1, 0};
    Token f = {TOK_CONSTANT, "foo", 0, 1};
    CHECK_TRUE(a == b);
    CHECK_TRUE(a != c);
    CHECK_TRUE(a != d);
    CHECK_TRUE(a != e);
    CHECK_TRUE(a != f);
}

TEST(ConfigTokenizer, SectionToken)
{
    std::stringstream ss("[Foo]");
    std::vector<Token> toksActual;
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toksActual, nullptr));
    std::vector<Token> toksExpect =
    {
        {TOK_SECTION, "Foo", 1, 1}
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
        {TOK_ANNOTATION, "@FOO", 1, 1}
    };
    CHECK_TRUE(toksExpect == toksActual);
}
