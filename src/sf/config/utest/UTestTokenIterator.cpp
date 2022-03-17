#include "sf/config/TokenIterator.hpp"
#include "sf/utest/UTest.hpp"

static const std::vector<Token> gToks =
{
    {Token::CONSTANT, "100", -1, -1},
    {Token::LABEL, "[foo]", -1, -1},
    {Token::IDENTIFIER, "foo", -1, -1},
    {Token::ANNOTATION, "@foo", -1, -1}
};

static const std::vector<Token> gToksNewlines =
{
    {Token::CONSTANT, "100", -1, -1},
    {Token::LABEL, "[foo]", -1, -1},
    {Token::NEWLINE, "(newline)", -1, -1},
    {Token::NEWLINE, "(newline)", -1, -1},
    {Token::IDENTIFIER, "foo", -1, -1},
    {Token::ANNOTATION, "@foo", -1, -1},
    {Token::NEWLINE, "(newline)", -1, -1},
    {Token::NEWLINE, "(newline)", -1, -1}
};

TEST_GROUP(TokenIterator)
{
};

TEST(TokenIterator, IdxAndSeek)
{
    TokenIterator it(gToks.begin(), gToks.end());
    CHECK_EQUAL(0, it.idx());
    it.seek(2);
    CHECK_EQUAL(2, it.idx());
    it.seek(gToks.size() + 1);
    CHECK_EQUAL(gToks.size(), it.idx());
}

TEST(TokenIterator, Eof)
{
    TokenIterator it(gToks.begin(), gToks.end());
    for (U32 i = 0; i < gToks.size(); ++i)
    {
        it.seek(i);
        CHECK_EQUAL(false, it.eof());
    }
    it.seek(gToks.size());
    CHECK_TRUE(it.eof());
}

TEST(TokenIterator, Tok)
{
    TokenIterator it(gToks.begin(), gToks.end());
    for (U32 i = 0; i < gToks.size(); ++i)
    {
        it.seek(i);
        CHECK_TRUE(it.tok() == gToks[i]);
    }
    it.seek(gToks.size());
    CHECK_TRUE(it.tok() == TokenIterator::NONE);
}

TEST(TokenIterator, Take)
{
    TokenIterator it(gToks.begin(), gToks.end());
    U32 takes = 0;
    while (!it.eof())
    {
        CHECK_EQUAL(takes, it.idx());
        CHECK_TRUE(it.take() == gToks[takes++]);
    }
    CHECK_EQUAL(gToks.size(), it.idx());
    CHECK_TRUE(it.take() == TokenIterator::NONE);
}

TEST(TokenIterator, TakeNewlines)
{
    TokenIterator it(gToksNewlines.begin(), gToksNewlines.end());
    it.seek(1);
    CHECK_TRUE(gToksNewlines[1] == it.take());
    CHECK_EQUAL(4, it.idx());
    it.seek(5);
    CHECK_TRUE(gToksNewlines[5] == it.take());
    CHECK_EQUAL(8, it.idx());
}

TEST(TokenIterator, TypeAndStr)
{
    TokenIterator it(gToks.begin(), gToks.end());
    while (!it.eof())
    {
        CHECK_EQUAL(gToks[it.idx()].type, it.type());
        CHECK_EQUAL(gToks[it.idx()].str, it.str());
        it.take();
    }
    CHECK_EQUAL(Token::NONE, it.type());
    CHECK_EQUAL("(none)", it.str());
}

TEST(TokenIterator, Eat)
{
    TokenIterator it(gToksNewlines.begin(), gToksNewlines.end());
    it.eat();
    CHECK_EQUAL(0, it.idx());
    it.seek(2);
    it.eat();
    CHECK_EQUAL(4, it.idx());
    it.seek(6);
    it.eat();
    CHECK_EQUAL(8, it.idx());
}

TEST(TokenIterator, Next)
{
    TokenIterator it(gToks.begin(), gToks.end());
    CHECK_EQUAL(0, it.next({Token::CONSTANT}));
    CHECK_EQUAL(1, it.next({Token::LABEL}));
    CHECK_EQUAL(2, it.next({Token::IDENTIFIER}));
    CHECK_EQUAL(3, it.next({Token::ANNOTATION}));
    CHECK_EQUAL(4, it.next({Token::LBRACE}));
    CHECK_EQUAL(2, it.next({Token::IDENTIFIER, Token::ANNOTATION}));
    CHECK_EQUAL(2, it.next({Token::IDENTIFIER, Token::LBRACE}));
    CHECK_EQUAL(4, it.next({Token::LBRACE, Token::RBRACE}));
    CHECK_EQUAL(0, it.idx());
}

TEST(TokenIterator, Slice)
{
    TokenIterator it(gToksNewlines.begin(), gToksNewlines.end());
    TokenIterator slice = it.slice(2, 5);

    CHECK_TRUE(gToksNewlines[2] == slice.tok());
    CHECK_EQUAL(0, slice.idx());
    slice.seek(slice.idx() + 1);
    CHECK_TRUE(gToksNewlines[3] == slice.tok());
    slice.seek(slice.idx() + 1);
    CHECK_TRUE(gToksNewlines[4] == slice.tok());
    slice.seek(slice.idx() + 1);
    CHECK_TRUE(slice.eof());
}

TEST(TokenIterator, SliceBackwardsBounds)
{
    TokenIterator it(gToks.begin(), gToks.end());
    TokenIterator slice = it.slice(2, 1);
    CHECK_TRUE(slice.eof());
}

TEST(TokenIterator, BackwardsBounds)
{
    TokenIterator it(gToks.end(), gToks.begin());
    CHECK_TRUE(it.eof());
}

TEST(TokenIterator, Size)
{
    TokenIterator it(gToksNewlines.begin(), gToksNewlines.end());
    CHECK_EQUAL(gToksNewlines.size(), it.size());
    TokenIterator slice = it.slice(2, 5);
    CHECK_EQUAL(3, slice.size());
}
