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
    CHECK_ERROR(kRes,
                StateMachineParser::parseStateVectorSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineParserStateVectorSectionErrors)
{
};

TEST(StateMachineParserStateVectorSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @READ_ONLY @READ_ONLY\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 20);
}

TEST(StateMachineParserStateVectorSectionErrors, MultipleAliasAnnotations)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar @ALIAS=baz\n");
    checkParseError(it, E_SMP_AL_MULT, 2, 20);
}

TEST(StateMachineParserStateVectorSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "@I32 foo\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserStateVectorSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32\n");
    checkParseError(it, E_SMP_EOF, 2, 4);
}

TEST(StateMachineParserStateVectorSectionErrors,
     UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 @foo\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 5);
}

TEST(StateMachineParserStateVectorSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @FOO\n");
    checkParseError(it, E_SMP_ANNOT, 2, 9);
}
