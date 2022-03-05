#include "sfa/sup/StateVectorParser.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(StateVectorParser)
{
};

TEST(StateVectorParser, NoRegions)
{
    // Parse state vector.
    TOKENIZE("");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(0, parse.regions.size());
}

TEST(StateVectorParser, EmptyRegion)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(0, parse.regions[0].elems.size());
}

TEST(StateVectorParser, MultipleEmptyRegions)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "[Bar]\n");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(0, parse.regions[0].elems.size());

    // Bar
    CHECK_EQUAL(toks[2], parse.regions[1].tokName);
    CHECK_EQUAL("Bar", parse.regions[1].plainName);
    CHECK_EQUAL(0, parse.regions[1].elems.size());
}

TEST(StateVectorParser, SelectSpecificRegions)
{
    // Parse state vector, but only regions `Foo` and `Baz`.
    TOKENIZE(
        "[Foo]\n"
        "[Bar]\n"
        "[Baz]\n"
        "[Qux]\n");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(
        StateVectorParser::parse(toks, parse, nullptr, {"Foo", "Baz"}));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(0, parse.regions[0].elems.size());

    // Baz
    CHECK_EQUAL(toks[4], parse.regions[1].tokName);
    CHECK_EQUAL("Baz", parse.regions[1].plainName);
    CHECK_EQUAL(0, parse.regions[1].elems.size());
}

TEST(StateVectorParser, RegionWithOneElement)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(1, parse.regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse.regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse.regions[0].elems[0].tokName);
}

TEST(StateVectorParser, RegionWithMultipleElements)
{
    // Parse state vector.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n");
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(1, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(2, parse.regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse.regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse.regions[0].elems[0].tokName);

    // bar
    CHECK_EQUAL(toks[5], parse.regions[0].elems[1].tokType);
    CHECK_EQUAL(toks[6], parse.regions[0].elems[1].tokName);
}

TEST(StateVectorParser, MultipleRegionsWithMultipleElements)
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
    StateVectorParser::Parse parse = {};
    CHECK_SUCCESS(StateVectorParser::parse(toks, parse, nullptr));

    // Expected number of regions was parsed.
    CHECK_EQUAL(2, parse.regions.size());

    // Foo
    CHECK_EQUAL(toks[0], parse.regions[0].tokName);
    CHECK_EQUAL("Foo", parse.regions[0].plainName);
    CHECK_EQUAL(2, parse.regions[0].elems.size());

    // foo
    CHECK_EQUAL(toks[2], parse.regions[0].elems[0].tokType);
    CHECK_EQUAL(toks[3], parse.regions[0].elems[0].tokName);

    // bar
    CHECK_EQUAL(toks[5], parse.regions[0].elems[1].tokType);
    CHECK_EQUAL(toks[6], parse.regions[0].elems[1].tokName);

    // Bar
    CHECK_EQUAL(toks[9], parse.regions[1].tokName);
    CHECK_EQUAL("Bar", parse.regions[1].plainName);
    CHECK_EQUAL(2, parse.regions[1].elems.size());

    // baz
    CHECK_EQUAL(toks[11], parse.regions[1].elems[0].tokType);
    CHECK_EQUAL(toks[12], parse.regions[1].elems[0].tokName);
    
    // qux
    CHECK_EQUAL(toks[14], parse.regions[1].elems[1].tokType);
    CHECK_EQUAL(toks[15], parse.regions[1].elems[1].tokName);
}
