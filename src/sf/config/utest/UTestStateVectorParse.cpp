#include "sf/config/StateVectorParse.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(
    const Vec<Token>& kToks,
    const Result kRes,
    const I32 kLineNum,
    const I32 kColNum,
    const Vec<String> kRegions = StateVectorParse::ALL_REGIONS)
{
    // Got expected return code from parser.
    Ref<const StateVectorParse> parse;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateVectorParse::parse(kToks, parse, &err, kRegions));

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

///////////////////////////////// Usage Tests //////////////////////////////////

TEST_GROUP(StateVectorParse)
{
};

TEST(StateVectorParse, NoRegions)
{
    // Parse state vector.
    TOKENIZE("");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(0, parse->regions.size());
}

TEST(StateVectorParse, EmptyRegion)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(0, parse->regions[0].elems.size());
}

TEST(StateVectorParse, MultipleEmptyRegions)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "[Bar]\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(0, parse->regions[0].elems.size());

    // Bar
    CHECK_EQUAL(toks[2], parse->regions[1].tokName);
    CHECK_EQUAL("Bar", parse->regions[1].plainName);
    CHECK_EQUAL(0, parse->regions[1].elems.size());
}

TEST(StateVectorParse, SelectSpecificRegions)
{
    // Parse state vector, but only regions `Foo` and `Baz`.
    TOKENIZE(
        "[Foo]\n"
        "[Bar]\n"
        "[Baz]\n"
        "[Qux]\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks,
                                           parse,
                                           nullptr,
                                           {"Foo", "Baz"}));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(0, parse->regions[0].elems.size());

    // Baz
    CHECK_EQUAL(toks[4], parse->regions[1].tokName);
    CHECK_EQUAL("Baz", parse->regions[1].plainName);
    CHECK_EQUAL(0, parse->regions[1].elems.size());
}

TEST(StateVectorParse, RegionWithOneElement)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(1, parse->regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse->regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse->regions[0].elems[0].tokName);
}

TEST(StateVectorParse, RegionWithMultipleElements)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(2, parse->regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse->regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse->regions[0].elems[0].tokName);

    // bar
    CHECK_EQUAL(toks[5], parse->regions[0].elems[1].tokType);
    CHECK_EQUAL(toks[6], parse->regions[0].elems[1].tokName);
}

TEST(StateVectorParse, MultipleRegionsWithMultipleElements)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "\n"
        "[Bar]\n"
        "bool baz\n"
        "U8 qux\n");
    Ref<const StateVectorParse> parse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse->regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse->regions[0].tokName);
    CHECK_EQUAL("Foo", parse->regions[0].plainName);
    CHECK_EQUAL(2, parse->regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse->regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse->regions[0].elems[0].tokName);

    // bar
    CHECK_EQUAL(toks[5], parse->regions[0].elems[1].tokType);
    CHECK_EQUAL(toks[6], parse->regions[0].elems[1].tokName);

    // Bar
    CHECK_EQUAL(toks[9], parse->regions[1].tokName);
    CHECK_EQUAL("Bar", parse->regions[1].plainName);
    CHECK_EQUAL(2, parse->regions[1].elems.size());

    // baz
    CHECK_EQUAL(toks[11], parse->regions[1].elems[0].tokType);
    CHECK_EQUAL(toks[12], parse->regions[1].elems[0].tokName);
    
    // qux
    CHECK_EQUAL(toks[14], parse->regions[1].elems[1].tokType);
    CHECK_EQUAL(toks[15], parse->regions[1].elems[1].tokName);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateVectorParseErrors)
{
};

TEST(StateVectorParseErrors, NonIdentifierForElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "@I32 foo\n");
    checkParseError(toks, E_SVP_ELEM_TYPE, 2, 1);
}

TEST(StateVectorParseErrors, MissingElementName)
{
    TOKENIZE(
        "[Foo]\n"
        "I32\n");
    checkParseError(toks, E_SVP_ELEM_NAME, 2, 1);
}

TEST(StateVectorParseErrors, NonIdentifierAfterElementType)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 @foo\n");
    checkParseError(toks, E_SVP_ELEM_NAME, 2, 5);
}

TEST(StateVectorParseErrors, SelectNonexistentRegion)
{
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n");
    checkParseError(toks, E_SVP_RGN, -1, -1, {"Bar"});
}
