#include <sstream>

#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator &kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    StateMachineParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParser::parseLocalSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineParserLocalSectionErrors)
{
};

TEST(StateMachineParserLocalSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY @READ_ONLY\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 24);
}

TEST(StateMachineParserLocalSectionErrors, MultipleLocalSections)
{
    // Got expected return code from parser.
    TOKENIZE(
        "[LOCAL]\n"
        "[LOCAL]\n");
    StateMachineParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(E_SMP_LOC_MULT, StateMachineParser::parse(toks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(2, err.lineNum);
    CHECK_EQUAL(1, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

TEST(StateMachineParserLocalSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo bar = 0\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32\n");
    checkParseError(it, E_SMP_EOF, 2, 4);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 @foo = 0\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 5);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo\n");
    checkParseError(it, E_SMP_EOF, 2, 8);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo @foo\n 0");
    checkParseError(it, E_SMP_LOC_OP, 2, 9);
}

TEST(StateMachineParserLocalSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo > 0\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 9);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo =\n");
    checkParseError(it, E_SMP_EOF, 2, 10);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = @foo\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 11);
}

TEST(StateMachineParserLocalSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @FOO\n");
    checkParseError(it, E_SMP_ANNOT, 2, 13);
}
