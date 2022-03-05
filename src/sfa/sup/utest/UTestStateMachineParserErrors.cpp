#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(const std::vector<Token>& kToks,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    StateMachineParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParser::parse(kToks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(0, parse.states.size());
    CHECK_TRUE(!parse.hasStateVectorSection);
    CHECK_TRUE(!parse.hasLocalSection);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineParserErrors)
{
};

TEST(StateMachineParserErrors, UnexpectedToken)
{
    TOKENIZE(
        "@foo\n"
        "[Foo]\n");
    checkParseError(toks, E_SMP_TOK, 1, 1);
}

TEST(StateMachineParserErrors, ErrorInStateVectorSection)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserErrors, ErrorInLocalSection)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserErrors, ErrorInStateSection)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_LAB, 2, 1);
}
