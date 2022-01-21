#include <sstream>

#include "sfa/sv/StateVectorParser.hpp"
#include "UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkStateVectorParserError(
    std::stringstream& kSs,
    const Result kRes,
    const U32 kLineNum,
    const U32 kColNum,
    std::vector<std::string> kSelectRegions = StateVectorParser::ALL_REGIONS)
{
    // Attempt to parse state vector config.
    std::shared_ptr<StateVectorParser::Config> config = nullptr;
    ConfigErrorInfo err;
    const Result res =
        StateVectorParser::parse(kSs, config, &err, kSelectRegions);

    // Got expected return code.
    CHECK_EQUAL(kRes, res);

    // Line and column numbers of offending token is correctly identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.msg.size() > 0);

    // Config pointer was not populated.
    POINTERS_EQUAL(nullptr, config.get());
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateVectorParserErrors)
{
};

TEST(StateVectorParserErrors, DupeSectionName)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Foo]\n" // Dupe section name
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_RGN_DUPE, 3, 1);
}

TEST(StateVectorParserErrors, InvalidSectionName)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGIONFoo]\n" // Erroneous section
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_SEC_NAME, 3, 1);
}

TEST(StateVectorParserErrors, UnexpectedTokenOutsideSection)
{
    std::stringstream ss(
        "64 " // Erroneous token
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Foo]\n"
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_TOK, 1, 1);
}

TEST(StateVectorParserErrors, SelectInvalidRegion)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Bar]\n"
        "I32 bar\n");
    // Select nonexistent region `Baz` for parsing.
    checkStateVectorParserError(ss, E_SVP_RGN_NAME, -1, -1, {"Baz"});
}

TEST(StateVectorParserErrors, EmptyConfig)
{
    std::stringstream ss;
    checkStateVectorParserError(ss, E_SVP_NO_RGNS, -1, -1);
}

TEST(StateVectorParserErrors, UnexpectedTokenInRegion)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo "
        "64\n" // Erroneous token
        "[REGION/Foo]\n"
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_RGN_TOK, 2, 9);
}

TEST(StateVectorParserErrors, InvalidElementType)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I3 " // Erroneous token
        "foo\n"
        "[REGION/Bar]\n"
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_ELEM_TYPE, 2, 1);
}

TEST(StateVectorParserErrors, NonIdentifierAfterElementType)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32\n" // Erroneous token
        "[REGION/Bar]\n"
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_ELEM_NAME, 3, 1);
}

TEST(StateVectorParserErrors, NothingAfterElementType)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Bar]\n"
        "I32\n"); // Erroneous token
    checkStateVectorParserError(ss, E_SVP_ELEM_NAME, 4, 1);
}

TEST(StateVectorParserErrors, DupeElementName)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Bar]\n"
        "I32 "
        "foo\n"); // Erroneous token
    checkStateVectorParserError(ss, E_SVP_ELEM_DUPE, 4, 5);
}

TEST(StateVectorParserErrors, EmptyRegionFirstInConfig)
{
    std::stringstream ss(
        "[REGION/Foo]\n" // Erroneous token
        "[REGION/Bar]\n"
        "I32 bar\n");
    checkStateVectorParserError(ss, E_SVP_RGN_EMPTY, 1, 1);
}

TEST(StateVectorParserErrors, EmptyRegionMiddleOfConfig)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Bar]\n" // Erroneous token
        "[REGION/Baz]\n"
        "I32 baz\n");
    checkStateVectorParserError(ss, E_SVP_RGN_EMPTY, 3, 1);
}

TEST(StateVectorParserErrors, EmptyRegionLastInConfig)
{
    std::stringstream ss(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "[REGION/Bar]\n"
        "I32 baz\n"
        "[REGION/Baz]\n"); // Erroneous token
    checkStateVectorParserError(ss, E_SVP_RGN_EMPTY, 5, 1);
}

TEST(StateVectorParserErrors, NonexistentInputFile)
{
    std::shared_ptr<StateVectorParser::Config> config = nullptr;
    CHECK_EQUAL(E_FILE,
                StateVectorParser::parse("foobar.baz", config, nullptr));
    POINTERS_EQUAL(nullptr, config.get());
}
