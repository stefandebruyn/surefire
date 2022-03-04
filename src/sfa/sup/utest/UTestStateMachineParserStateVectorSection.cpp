#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(StateMachineParserStateVectorSection)
{
};

TEST(StateMachineParserStateVectorSection, Empty)
{
    TOKENIZE("[STATE_VECTOR]");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserStateVectorSection, EmptyWithNewlines)
{
    TOKENIZE("[STATE_VECTOR]\n\n\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserStateVectorSection, OneElement)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(false, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, ReadOnlyAnnotation)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @READ_ONLY\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(true, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, AliasAnnotation)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.svElems[0].tokAlias == toks[4]);
    CHECK_EQUAL("bar", parse.svElems[0].alias);
    CHECK_EQUAL(false, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, MultipleAnnotations)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar @READ_ONLY\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.svElems[0].tokAlias == toks[4]);
    CHECK_EQUAL("bar", parse.svElems[0].alias);
    CHECK_EQUAL(true, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, MultipleElements)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(false, parse.svElems[0].readOnly);

    CHECK_TRUE(parse.svElems[1].tokType == toks[5]);
    CHECK_TRUE(parse.svElems[1].tokName == toks[6]);
    CHECK_EQUAL(0, parse.svElems[1].alias.size());
    CHECK_EQUAL(false, parse.svElems[1].readOnly);

    CHECK_TRUE(parse.svElems[2].tokType == toks[8]);
    CHECK_TRUE(parse.svElems[2].tokName == toks[9]);
    CHECK_EQUAL(0, parse.svElems[2].alias.size());
    CHECK_EQUAL(false, parse.svElems[2].readOnly);
}

TEST(StateMachineParserStateVectorSection, AllElementTypes)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
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
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(11, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(false, parse.svElems[0].readOnly);

    CHECK_TRUE(parse.svElems[1].tokType == toks[5]);
    CHECK_TRUE(parse.svElems[1].tokName == toks[6]);
    CHECK_EQUAL(0, parse.svElems[1].alias.size());
    CHECK_EQUAL(false, parse.svElems[1].readOnly);

    CHECK_TRUE(parse.svElems[2].tokType == toks[8]);
    CHECK_TRUE(parse.svElems[2].tokName == toks[9]);
    CHECK_EQUAL(0, parse.svElems[2].alias.size());
    CHECK_EQUAL(false, parse.svElems[2].readOnly);

    CHECK_TRUE(parse.svElems[3].tokType == toks[11]);
    CHECK_TRUE(parse.svElems[3].tokName == toks[12]);
    CHECK_EQUAL(0, parse.svElems[3].alias.size());
    CHECK_EQUAL(false, parse.svElems[3].readOnly);

    CHECK_TRUE(parse.svElems[4].tokType == toks[14]);
    CHECK_TRUE(parse.svElems[4].tokName == toks[15]);
    CHECK_EQUAL(0, parse.svElems[4].alias.size());
    CHECK_EQUAL(false, parse.svElems[4].readOnly);

    CHECK_TRUE(parse.svElems[5].tokType == toks[17]);
    CHECK_TRUE(parse.svElems[5].tokName == toks[18]);
    CHECK_EQUAL(0, parse.svElems[5].alias.size());
    CHECK_EQUAL(false, parse.svElems[5].readOnly);

    CHECK_TRUE(parse.svElems[6].tokType == toks[20]);
    CHECK_TRUE(parse.svElems[6].tokName == toks[21]);
    CHECK_EQUAL(0, parse.svElems[6].alias.size());
    CHECK_EQUAL(false, parse.svElems[6].readOnly);

    CHECK_TRUE(parse.svElems[7].tokType == toks[23]);
    CHECK_TRUE(parse.svElems[7].tokName == toks[24]);
    CHECK_EQUAL(0, parse.svElems[7].alias.size());
    CHECK_EQUAL(false, parse.svElems[7].readOnly);

    CHECK_TRUE(parse.svElems[8].tokType == toks[26]);
    CHECK_TRUE(parse.svElems[8].tokName == toks[27]);
    CHECK_EQUAL(0, parse.svElems[8].alias.size());
    CHECK_EQUAL(false, parse.svElems[8].readOnly);

    CHECK_TRUE(parse.svElems[9].tokType == toks[29]);
    CHECK_TRUE(parse.svElems[9].tokName == toks[30]);
    CHECK_EQUAL(0, parse.svElems[9].alias.size());
    CHECK_EQUAL(false, parse.svElems[9].readOnly);

    CHECK_TRUE(parse.svElems[10].tokType == toks[32]);
    CHECK_TRUE(parse.svElems[10].tokName == toks[33]);
    CHECK_EQUAL(0, parse.svElems[10].alias.size());
    CHECK_EQUAL(false, parse.svElems[10].readOnly);
}

TEST(StateMachineParserStateVectorSection, MultipleElementsWithAnnotations)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "F64 bar @READ_ONLY\n"
        "bool baz @ALIAS=qux\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(false, parse.svElems[0].readOnly);

    CHECK_TRUE(parse.svElems[1].tokType == toks[5]);
    CHECK_TRUE(parse.svElems[1].tokName == toks[6]);
    CHECK_EQUAL(0, parse.svElems[1].alias.size());
    CHECK_EQUAL(true, parse.svElems[1].readOnly);

    CHECK_TRUE(parse.svElems[2].tokType == toks[9]);
    CHECK_TRUE(parse.svElems[2].tokName == toks[10]);
    CHECK_TRUE(parse.svElems[2].tokAlias == toks[11]);
    CHECK_EQUAL("qux", parse.svElems[2].alias);
    CHECK_EQUAL(false, parse.svElems[2].readOnly);
}
