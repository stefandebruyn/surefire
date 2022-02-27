#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

#include <sstream>

/////////////////////////////////// Helpers ////////////////////////////////////

#define TOKENIZE(kStr)                                                         \
    std::stringstream ss(kStr);                                                \
    std::vector<Token> toks;                                                   \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toks, nullptr));               \
    TokenIterator it(toks.begin(), toks.end());

static void checkParseError(TokenIterator &kIt,
                            const StateVector& kSv,
                            StateMachineParser::Parse& kParse,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Attempt to parse local section.
    ConfigErrorInfo err;
    const Result res =
        StateMachineParser::parseStateVectorSection(kIt, kSv, kParse, &err);

    // Got expected return code.
    CHECK_ERROR(kRes, res);

    // Line and column numbers of offending token are correctly identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineParserStateVectorSection)
{
};

TEST(StateMachineParserStateVectorSection, Empty)
{
    TOKENIZE("[STATE_VECTOR]");
    StateMachineParser::Parse parse;
    StateVector sv;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}
