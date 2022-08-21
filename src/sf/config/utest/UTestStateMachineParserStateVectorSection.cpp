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
/// @file  sf/config/utest/UTestStateMachineParserStateVectorSection.hpp
/// @brief Unit tests for StateMachineParser parsing state vector sections.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that parsing a state machine generates a certain error.
///
/// @param[in] kIt       State machine config to parse.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error column number.
/// @param[in] kColNum   Expected error line number.
///
static void checkParseError(TokenIterator &kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    Vec<StateMachineParse::StateVectorElementParse> parse;
    ErrorInfo err;
    TokenIterator itCpy = kIt;
    CHECK_ERROR(kRes, StateMachineParser::parseStateVectorSection(kIt,
                                                                 parse,
                                                                 &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineParser::parseStateVectorSection(itCpy,
                                                                 parse,
                                                                 nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for StateMachineParser parsing state vector sections.
///
TEST_GROUP(StateMachineParseStateVectorSection)
{
};

///
/// @test An empty state vector section is parsed correctly.
///
TEST(StateMachineParseStateVectorSection, Empty)
{
    TOKENIZE("[state_vector]");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

///
/// @test An empty state vector section containing newlines is parsed correctly.
///
TEST(StateMachineParseStateVectorSection, EmptyWithNewlines)
{
    TOKENIZE("[state_vector]\n\n\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

///
/// @test A state vector section containing one element is parsed correctly.
///
TEST(StateMachineParseStateVectorSection, OneElement)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    CHECK_EQUAL(false, parse[0].readOnly);
}

///
/// @test A read-only annotation is parsed correctly.
///
TEST(StateMachineParseStateVectorSection, ReadOnlyAnnotation)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @read_only\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    CHECK_EQUAL(true, parse[0].readOnly);
}

///
/// @test An alias annotation is parsed correctly.
///
TEST(StateMachineParseStateVectorSection, AliasAnnotation)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias bar\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_EQUAL(toks[2], parse[0].tokType);
    CHECK_EQUAL(toks[3], parse[0].tokName);
    CHECK_EQUAL(toks[5], parse[0].tokAlias);
    CHECK_EQUAL(false, parse[0].readOnly);
}

///
/// @test Multiple annotations on the same element are parsed correctly.
///
TEST(StateMachineParseStateVectorSection, MultipleAnnotations)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias bar @read_only\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_EQUAL(toks[2], parse[0].tokType);
    CHECK_EQUAL(toks[3], parse[0].tokName);
    CHECK_EQUAL(toks[5], parse[0].tokAlias);
    CHECK_EQUAL(true, parse[0].readOnly);
}

///
/// @test A state vector section containing multiple elements is parsed
/// correctly.
///
TEST(StateMachineParseStateVectorSection, MultipleElements)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    CHECK_EQUAL(false, parse[0].readOnly);

    CHECK_TRUE(parse[1].tokType == toks[5]);
    CHECK_TRUE(parse[1].tokName == toks[6]);
    CHECK_EQUAL(false, parse[1].readOnly);

    CHECK_TRUE(parse[2].tokType == toks[8]);
    CHECK_TRUE(parse[2].tokName == toks[9]);
    CHECK_EQUAL(false, parse[2].readOnly);
}

///
/// @test All element types are parsed correctly.
///
TEST(StateMachineParseStateVectorSection, AllElementTypes)
{
    TOKENIZE(
        "[state_vector]\n"
        "I8 a\n"
        "I16 b\n"
        "I32 c\n"
        "I64 d\n"
        "U8 e\n"
        "U16 f\n"
        "U32 g\n"
        "U64 h\n"
        "F32 i\n"
        "F64 j\n"
        "bool k\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(11, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    CHECK_EQUAL(false, parse[0].readOnly);

    CHECK_TRUE(parse[1].tokType == toks[5]);
    CHECK_TRUE(parse[1].tokName == toks[6]);
    CHECK_EQUAL(false, parse[1].readOnly);

    CHECK_TRUE(parse[2].tokType == toks[8]);
    CHECK_TRUE(parse[2].tokName == toks[9]);
    CHECK_EQUAL(false, parse[2].readOnly);

    CHECK_TRUE(parse[3].tokType == toks[11]);
    CHECK_TRUE(parse[3].tokName == toks[12]);
    CHECK_EQUAL(false, parse[3].readOnly);

    CHECK_TRUE(parse[4].tokType == toks[14]);
    CHECK_TRUE(parse[4].tokName == toks[15]);
    CHECK_EQUAL(false, parse[4].readOnly);

    CHECK_TRUE(parse[5].tokType == toks[17]);
    CHECK_TRUE(parse[5].tokName == toks[18]);
    CHECK_EQUAL(false, parse[5].readOnly);

    CHECK_TRUE(parse[6].tokType == toks[20]);
    CHECK_TRUE(parse[6].tokName == toks[21]);
    CHECK_EQUAL(false, parse[6].readOnly);

    CHECK_TRUE(parse[7].tokType == toks[23]);
    CHECK_TRUE(parse[7].tokName == toks[24]);
    CHECK_EQUAL(false, parse[7].readOnly);

    CHECK_TRUE(parse[8].tokType == toks[26]);
    CHECK_TRUE(parse[8].tokName == toks[27]);
    CHECK_EQUAL(false, parse[8].readOnly);

    CHECK_TRUE(parse[9].tokType == toks[29]);
    CHECK_TRUE(parse[9].tokName == toks[30]);
    CHECK_EQUAL(false, parse[9].readOnly);

    CHECK_TRUE(parse[10].tokType == toks[32]);
    CHECK_TRUE(parse[10].tokName == toks[33]);
    CHECK_EQUAL(false, parse[10].readOnly);
}

///
/// @test Multiple elements with annotations are parsed correctly.
///
TEST(StateMachineParseStateVectorSection, MultipleElementsWithAnnotations)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo\n"
        "F64 bar @read_only\n"
        "bool baz @alias qux\n");
    Vec<StateMachineParse::StateVectorElementParse> parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    CHECK_EQUAL(false, parse[0].readOnly);

    CHECK_TRUE(parse[1].tokType == toks[5]);
    CHECK_TRUE(parse[1].tokName == toks[6]);
    CHECK_EQUAL(true, parse[1].readOnly);

    CHECK_TRUE(parse[2].tokType == toks[9]);
    CHECK_TRUE(parse[2].tokName == toks[10]);
    CHECK_TRUE(parse[2].tokAlias == toks[12]);
    CHECK_EQUAL(false, parse[2].readOnly);
}

///////////////////////////////// Error Tests //////////////////////////////////

///
/// @brief Unit tests for StateMachineParser parsing state vector sections with
/// errors.
///
TEST_GROUP(StateMachineParseStateVectorSectionErrors)
{
};

///
/// @test Multiple read-only annotations on the same element generate an error.
///
TEST(StateMachineParseStateVectorSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @read_only @read_only\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 20);
}

///
/// @test Multiple alias annotations on the same element generate an error.
///
TEST(StateMachineParseStateVectorSectionErrors, MultipleAliasAnnotations)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias bar @alias baz\n");
    checkParseError(it, E_SMP_AL_MULT, 2, 20);
}

///
/// @test An unexpected token after an alias generates an error.
///
TEST(StateMachineParseStateVectorSectionErrors, UnexpectedTokenAfterAlias)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias 10\n");
    checkParseError(it, E_SMP_ALIAS, 2, 9);
}

///
/// @test No tokens after an alias annotation generates an error.
///
TEST(StateMachineParseStateVectorSectionErrors, EofAfterAlias)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias\n");
    checkParseError(it, E_SMP_ALIAS, 2, 9);
}

///
/// @test A non-identifier token where an element type is expected generates an
/// error.
///
TEST(StateMachineParseStateVectorSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[state_vector]\n"
        "@I32 foo\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

///
/// @test No tokens after an element type generates an error.
///
TEST(StateMachineParseStateVectorSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

///
/// @test A non-identifier token where an element name is expected generates an
/// error.
///
TEST(StateMachineParseStateVectorSectionErrors, UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 @foo\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

///
/// @test An unknown annotation generates an error.
///
TEST(StateMachineParseStateVectorSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @foo\n");
    checkParseError(it, E_SMP_ANNOT, 2, 9);
}
