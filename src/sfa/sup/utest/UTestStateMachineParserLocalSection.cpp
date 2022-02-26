#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

#include <sstream>

#define TOKENIZE(kStr)                                                         \
    std::stringstream ss(kStr);                                                \
    std::vector<Token> toks;                                                   \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(ss, toks, nullptr));               \
    TokenIterator it(toks.begin(), toks.end());

TEST_GROUP(StateMachineParserLocalSection)
{
};

TEST(StateMachineParserLocalSection, Simple)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0 @READ_ONLY\n"
        "bool baz = false @ALIAS=qux @READ_ONLY\n");
    StateMachineParser::Parse parse;
    StateVector sv;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, sv, parse, nullptr));

    CHECK_EQUAL(3, parse.localElems.size());

    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL("", parse.localElems[0].alias);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL("", parse.localElems[1].alias);
    CHECK_EQUAL(true, parse.localElems[1].readOnly);

    CHECK_TRUE(parse.localElems[2].tokType == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[14]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[16]);
    CHECK_EQUAL("qux", parse.localElems[2].alias);
    CHECK_EQUAL(true, parse.localElems[2].readOnly);
}
