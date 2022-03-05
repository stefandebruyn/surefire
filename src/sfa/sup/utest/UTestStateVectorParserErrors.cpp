#include "sfa/sup/StateVectorParser.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(
    const std::vector<Token>& kToks,
    const Result kRes,
    const I32 kLineNum,
    const I32 kColNum,
    const std::vector<std::string> kRgns = StateVectorParser::ALL_REGIONS)
{
    // Got expected return code from parser.
    StateVectorParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(kRes, StateVectorParser::parse(kToks, parse, &err, kRgns));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_EQUAL(0, parse.regions.size());
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateVectorParserErrors)
{
};

TEST(StateVectorParserErrors, NonIdentifierForElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "@I32 foo\n");
    checkParseError(toks, E_SVP_ELEM_TYPE, 2, 1);
}

TEST(StateVectorParserErrors, MissingElementName)
{
    TOKENIZE(
        "[Foo]\n"
        "I32\n");
    checkParseError(toks, E_SVP_ELEM_NAME, 2, 1);
}

TEST(StateVectorParserErrors, NonIdentifierAfterElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 @foo\n");
    checkParseError(toks, E_SVP_ELEM_NAME, 2, 5);
}

TEST(StateVectorParserErrors, SelectNonexistentRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n");
    checkParseError(toks, E_SVP_RGN, -1, -1, {"Bar"});
}
