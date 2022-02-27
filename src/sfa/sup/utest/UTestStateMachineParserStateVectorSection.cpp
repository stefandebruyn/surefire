#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/sup/StateVectorParser.hpp"
#include "sfa/utest/UTest.hpp"

#include <sstream>

/////////////////////////////////// Helpers ////////////////////////////////////

#define INIT_SV(kSrc)                                                          \
    std::stringstream svSs(kSrc);                                              \
    std::shared_ptr<StateVectorParser::Config> svConfig = nullptr;             \
    CHECK_SUCCESS(StateVectorParser::parse(svSs, svConfig, nullptr));          \
    StateVector sv;                                                            \
    CHECK_SUCCESS(StateVector::create(svConfig->get(), sv));                   \

#define TOKENIZE_SM(kSrc)                                                      \
    std::stringstream smSs(kSrc);                                              \
    std::vector<Token> toks;                                                   \
    CHECK_SUCCESS(ConfigTokenizer::tokenize(smSs, toks, nullptr));             \
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
    TOKENIZE_SM("[STATE_VECTOR]");
    StateMachineParser::Parse parse;
    StateVector sv;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserStateVectorSection, OneElement)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(false, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, ReadOnlyAnnotation)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo @READ_ONLY\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.svElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.svElems[0].tokName == toks[3]);
    CHECK_EQUAL(0, parse.svElems[0].alias.size());
    CHECK_EQUAL(true, parse.svElems[0].readOnly);
}

TEST(StateMachineParserStateVectorSection, AliasAnnotation)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

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
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar @READ_ONLY\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

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
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

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

TEST(StateMachineParserStateVectorSection, MultipleElementsWithAnnotations)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "F64 bar @READ_ONLY\n"
        "bool baz @ALIAS=qux\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseStateVectorSection(it, sv, parse, nullptr));

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

TEST(StateMachineParserStateVectorSection, ErrorRedundantReadOnlyAnnotation)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo @READ_ONLY @READ_ONLY\n");
    StateMachineParser::Parse parse;
    checkParseError(it, sv, parse, E_SMP_RO_MULT, 2, 20);
}

TEST(StateMachineParserStateVectorSection, ErrorMultipleAliasAnnotations)
{
    INIT_SV(
        "[REGION/Foo]\n"
        "I32 foo");
    TOKENIZE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=bar @ALIAS=baz\n");
    StateMachineParser::Parse parse;
    checkParseError(it, sv, parse, E_SMP_AL_MULT, 2, 20);
}
