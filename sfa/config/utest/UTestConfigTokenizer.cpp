#include <sstream>

#include "sfa/config/ConfigTokenizer.hpp"
#include "UTest.hpp"

TEST_GROUP(ConfigTokenizer)
{
};

TEST(ConfigTokenizer, TokenEquivalence)
{
    Token a = {Token::CONSTANT, "foo", 0, 0};
    Token b = {Token::CONSTANT, "foo", 0, 0};
    Token c = {Token::LABEL,    "foo", 0, 0};
    Token d = {Token::CONSTANT, "bar", 0, 0};
    Token e = {Token::CONSTANT, "foo", 1, 0};
    Token f = {Token::CONSTANT, "foo", 0, 1};
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
        {Token::SECTION, "Foo", 1, 1}
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
        {Token::ANNOTATION, "@FOO", 1, 1}
    };
    CHECK_TRUE(toksExpect == toksActual);
}
