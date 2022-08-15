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
/// @file  sf/config/utest/UTestTokenIterator.hpp
/// @brief Unit tests for TokenIterator.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/TokenIterator.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

///
/// @brief Short token sequence for use in tests.
///
static const Vec<Token> gToks =
{
    {Token::CONSTANT, "100", -1, -1},
    {Token::LABEL, "[foo]", -1, -1},
    {Token::IDENTIFIER, "foo", -1, -1},
    {Token::ANNOTATION, "@foo", -1, -1}
};

///
/// @brief Token sequence with newlines for use in tests.
///
static const Vec<Token> gToksNewlines =
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

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for TokenIterator.
///
TEST_GROUP(TokenIterator)
{
};

///
/// @test TokenIterator::idx() and TokenIterator::seek() work correctly.
///
TEST(TokenIterator, IdxAndSeek)
{
    TokenIterator it(gToks.begin(), gToks.end());
    CHECK_EQUAL(0, it.idx());
    it.seek(2);
    CHECK_EQUAL(2, it.idx());
    it.seek(gToks.size() + 1);
    CHECK_EQUAL(gToks.size(), it.idx());
}

///
/// @test TokenIterator::eof() works correctly.
///
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

///
/// @test TokenIterator::tok() works correctly.
///
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

///
/// @test TokenIterator::take() works correctly.
///
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

///
/// @test TokenIterator::take() with newlines works correctly.
///
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

///
/// @test TokenIterator::type() and TokenIterator::str() work correctly.
///
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

///
/// @test TokenIterator::eat() works correctly.
///
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

///
/// @test TokenIterator::next() works correctly.
///
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

///
/// @test TokenIterator::slice() works correctly.
///
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

///
/// @test TokenIterator::slice() with backwards bounds works correctly.
///
TEST(TokenIterator, SliceBackwardsBounds)
{
    TokenIterator it(gToks.begin(), gToks.end());
    TokenIterator slice = it.slice(2, 1);
    CHECK_TRUE(slice.eof());
}

///
/// @test A TokenIterator with backwards bounds does not iterate.
///
TEST(TokenIterator, BackwardsBounds)
{
    TokenIterator it(gToks.end(), gToks.begin());
    CHECK_TRUE(it.eof());
}

///
/// @test TokenIterator::size() works correctly.
///
TEST(TokenIterator, Size)
{
    TokenIterator it(gToksNewlines.begin(), gToksNewlines.end());
    CHECK_EQUAL(gToksNewlines.size(), it.size());
    TokenIterator slice = it.slice(2, 5);
    CHECK_EQUAL(3, slice.size());
}
