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
                            StateMachineParser::Parse& kParse,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Attempt to parse local section.
    ConfigErrorInfo err;
    const Result res =
        StateMachineParser::parseLocalSection(kIt, kParse, &err);

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

TEST_GROUP(StateMachineParserLocalSection)
{
};

TEST(StateMachineParserLocalSection, Empty)
{
    TOKENIZE("[LOCAL]");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserLocalSection, OneElement)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);
}

TEST(StateMachineParserLocalSection, ReadOnlyAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    CHECK_EQUAL(1, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(true, parse.localElems[0].readOnly);
}

TEST(StateMachineParserLocalSection, MultipleElements)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0\n"
        "bool baz = false\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL(false, parse.localElems[1].readOnly);

    CHECK_TRUE(parse.localElems[2].tokType == toks[12]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[15]);
    CHECK_EQUAL(false, parse.localElems[2].readOnly);
}

TEST(StateMachineParserLocalSection, MultipleElementsWithAnnotations)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0 @READ_ONLY\n"
        "bool baz = false @READ_ONLY\n");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    CHECK_EQUAL(3, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL(true, parse.localElems[1].readOnly);

    CHECK_TRUE(parse.localElems[2].tokType == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[14]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[16]);
    CHECK_EQUAL(true, parse.localElems[2].readOnly);
}

TEST(StateMachineParserLocalSection, ErrorRedundantReadOnlyAnnotation)
{
    TOKENIZE("[LOCAL] I32 foo = 0 @READ_ONLY @READ_ONLY");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_RO_MULT, 1, 32);
}

TEST(StateMachineParserLocalSection, ErrorMultipleLocalSections)
{
    // First local section is successfully parsed.
    TOKENIZE("[LOCAL] [LOCAL]");
    StateMachineParser::Parse parse;
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Second local section produces an error.
    checkParseError(it, parse, E_SMP_LOC_MULT, 1, 9);
}

TEST(StateMachineParserLocalSection, ErrorExpectedElementType)
{
    TOKENIZE("[LOCAL] @foo");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_ELEM_TYPE, 1, 9);
}

TEST(StateMachineParserLocalSection, ErrorInvalidElementType)
{
    TOKENIZE("[LOCAL] I33 foo = 0");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_ELEM_TYPE, 1, 9);
}

TEST(StateMachineParserLocalSection, ErrorReservedElementName)
{
    TOKENIZE("[LOCAL] I32 STATE_VECTOR = 0");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_NAME_RSVD, 1, 13);
}

TEST(StateMachineParserLocalSection, ErrorReuseLocalElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 foo = 0\n");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_NAME_DUPE, 3, 5);
}

TEST(StateMachineParserLocalSection, ErrorReuseStateVectorElementName)
{
    TOKENIZE("[LOCAL] I32 foo = 0");
    StateMachineParser::Parse parse;
    StateMachineParser::StateVectorElementParse svElemParse;
    svElemParse.tokName = {Token::IDENTIFIER, "foo", -1, -1};
    parse.svElems.push_back(svElemParse);
    checkParseError(it, parse, E_SMP_NAME_DUPE, 1, 13);
}

TEST(StateMachineParserLocalSection, ErrorReuseStateVectorElementAlias)
{
    TOKENIZE("[LOCAL] I32 foo = 0");
    StateMachineParser::Parse parse;
    StateMachineParser::StateVectorElementParse svElemParse;
    svElemParse.alias = "foo";
    parse.svElems.push_back(svElemParse);
    checkParseError(it, parse, E_SMP_NAME_DUPE, 1, 13);
}

TEST(StateMachineParserLocalSection, ErrorReuseStateName)
{
    TOKENIZE("[LOCAL] I32 foo = 0");
    StateMachineParser::Parse parse;
    StateMachineParser::StateParse stateParse;
    stateParse.tokName = {Token::IDENTIFIER, "foo", -1, -1};
    parse.states.push_back(stateParse);
    checkParseError(it, parse, E_SMP_NAME_DUPE, 1, 13);
}

TEST(StateMachineParserLocalSection, ErrorMissingElementAssignment)
{
    TOKENIZE("[LOCAL] I32 foo");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_LOC_OP, 1, 13);
}

TEST(StateMachineParserLocalSection, ErrorAssignmentWrongOperator)
{
    TOKENIZE("[LOCAL] I32 foo > 0");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_LOC_OP, 1, 17);
}

TEST(StateMachineParserLocalSection, ErrorUnknownAnnotation)
{
    TOKENIZE("[LOCAL] I32 foo = 0 @FOO");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_ANNOT, 1, 21);
}

TEST(StateMachineParserLocalSection, ErrorMissingElementNameEndOfFile)
{
    TOKENIZE("[LOCAL] I32");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_ELEM_NAME, 1, 9);
}

TEST(StateMachineParserLocalSection, ErrorMissingElementNameUnexpectedToken)
{
    TOKENIZE("[LOCAL] I32 @foo");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_ELEM_NAME, 1, 9);
}

TEST(StateMachineParserLocalSection, ErrorMissingElementValueEndOfFile)
{
    TOKENIZE("[LOCAL] I32 foo =");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_LOC_VAL, 1, 17);
}

TEST(StateMachineParserLocalSection, ErrorMissingElementValueUnexpectedToken)
{
    TOKENIZE("[LOCAL] I32 foo = @foo");
    StateMachineParser::Parse parse;
    checkParseError(it, parse, E_SMP_LOC_VAL, 1, 17);
}
