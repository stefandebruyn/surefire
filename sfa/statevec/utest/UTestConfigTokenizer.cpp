#include <sstream>

#include "sfa/ConfigTokenizer.hpp"
#include "CppUTest/TestHarness.h"

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
    std::stringstream ss("[FOO]");
    std::vector<Token> toksActual;
    Result res = Tokenizer::tokenize(ss, toksActual, nullptr);
    CHECK_EQUAL(SUCCESS, res);
    std::vector<Token> toksExpect =
    {
        {TOK_SECTION, 0, "FOO"}
    };
    CHECK_TRUE(toksExpect == toksActual);
}
