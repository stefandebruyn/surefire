#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator &kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    StateMachineParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParser::parseLocalSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

///////////////////////////////// Usage Tests //////////////////////////////////

TEST_GROUP(StateMachineParserLocalSection)
{
};

TEST(StateMachineParserLocalSection, Empty)
{
    // Parse local section.
    TOKENIZE("[LOCAL]");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserLocalSection, EmptyWithNewlines)
{
    TOKENIZE("[LOCAL]\n\n\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));
    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParserLocalSection, OneElement)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // foo
    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);
}

TEST(StateMachineParserLocalSection, ReadOnlyAnnotation)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // foo
    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(true, parse.localElems[0].readOnly);
}

TEST(StateMachineParserLocalSection, MultipleElements)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0\n"
        "BOOL baz = FALSE\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // foo
    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    // bar
    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL(false, parse.localElems[1].readOnly);

    // baz
    CHECK_TRUE(parse.localElems[2].tokType == toks[12]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[15]);
    CHECK_EQUAL(false, parse.localElems[2].readOnly);
}

TEST(StateMachineParserLocalSection, MultipleElementsWithAnnotations)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0 @READ_ONLY\n"
        "BOOL baz = FALSE @READ_ONLY\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // foo
    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    // bar
    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL(true, parse.localElems[1].readOnly);

    // baz
    CHECK_TRUE(parse.localElems[2].tokType == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[14]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[16]);
    CHECK_EQUAL(true, parse.localElems[2].readOnly);
}

TEST(StateMachineParserLocalSection, AllElementTypes)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I8 a = 0\n"
        "I16 b = 0\n"
        "I32 c = 0\n"
        "I64 d = 0\n"
        "U8 e = 0\n"
        "U16 f = 0\n"
        "U32 g = 0\n"
        "U64 h = 0\n"
        "F32 i = 0.0\n"
        "F64 j = 0.0\n"
        "BOOL k = FALSE\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(
        StateMachineParser::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(11, parse.localElems.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // a
    CHECK_TRUE(parse.localElems[0].tokType == toks[2]);
    CHECK_TRUE(parse.localElems[0].tokName == toks[3]);
    CHECK_TRUE(parse.localElems[0].tokInitVal == toks[5]);
    CHECK_EQUAL(false, parse.localElems[0].readOnly);

    // b
    CHECK_TRUE(parse.localElems[1].tokType == toks[7]);
    CHECK_TRUE(parse.localElems[1].tokName == toks[8]);
    CHECK_TRUE(parse.localElems[1].tokInitVal == toks[10]);
    CHECK_EQUAL(false, parse.localElems[1].readOnly);

    // c
    CHECK_TRUE(parse.localElems[2].tokType == toks[12]);
    CHECK_TRUE(parse.localElems[2].tokName == toks[13]);
    CHECK_TRUE(parse.localElems[2].tokInitVal == toks[15]);
    CHECK_EQUAL(false, parse.localElems[2].readOnly);

    // d
    CHECK_TRUE(parse.localElems[3].tokType == toks[17]);
    CHECK_TRUE(parse.localElems[3].tokName == toks[18]);
    CHECK_TRUE(parse.localElems[3].tokInitVal == toks[20]);
    CHECK_EQUAL(false, parse.localElems[3].readOnly);

    // e
    CHECK_TRUE(parse.localElems[4].tokType == toks[22]);
    CHECK_TRUE(parse.localElems[4].tokName == toks[23]);
    CHECK_TRUE(parse.localElems[4].tokInitVal == toks[25]);
    CHECK_EQUAL(false, parse.localElems[4].readOnly);

    // f
    CHECK_TRUE(parse.localElems[5].tokType == toks[27]);
    CHECK_TRUE(parse.localElems[5].tokName == toks[28]);
    CHECK_TRUE(parse.localElems[5].tokInitVal == toks[30]);
    CHECK_EQUAL(false, parse.localElems[5].readOnly);

    // g
    CHECK_TRUE(parse.localElems[6].tokType == toks[32]);
    CHECK_TRUE(parse.localElems[6].tokName == toks[33]);
    CHECK_TRUE(parse.localElems[6].tokInitVal == toks[35]);
    CHECK_EQUAL(false, parse.localElems[6].readOnly);

    // h
    CHECK_TRUE(parse.localElems[7].tokType == toks[37]);
    CHECK_TRUE(parse.localElems[7].tokName == toks[38]);
    CHECK_TRUE(parse.localElems[7].tokInitVal == toks[40]);
    CHECK_EQUAL(false, parse.localElems[7].readOnly);

    // i
    CHECK_TRUE(parse.localElems[8].tokType == toks[42]);
    CHECK_TRUE(parse.localElems[8].tokName == toks[43]);
    CHECK_TRUE(parse.localElems[8].tokInitVal == toks[45]);
    CHECK_EQUAL(false, parse.localElems[8].readOnly);

    // j
    CHECK_TRUE(parse.localElems[9].tokType == toks[47]);
    CHECK_TRUE(parse.localElems[9].tokName == toks[48]);
    CHECK_TRUE(parse.localElems[9].tokInitVal == toks[50]);
    CHECK_EQUAL(false, parse.localElems[9].readOnly);

    // k
    CHECK_TRUE(parse.localElems[10].tokType == toks[52]);
    CHECK_TRUE(parse.localElems[10].tokName == toks[53]);
    CHECK_TRUE(parse.localElems[10].tokInitVal == toks[55]);
    CHECK_EQUAL(false, parse.localElems[10].readOnly);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParserLocalSectionErrors)
{
};

TEST(StateMachineParserLocalSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY @READ_ONLY\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 24);
}

TEST(StateMachineParserLocalSectionErrors, MultipleLocalSections)
{
    // Got expected return code from parser.
    TOKENIZE(
        "[LOCAL]\n"
        "[LOCAL]\n");
    StateMachineParser::Parse parse = {};
    ConfigErrorInfo err;
    CHECK_ERROR(E_SMP_LOC_MULT, StateMachineParser::parse(toks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(2, err.lineNum);
    CHECK_EQUAL(1, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

TEST(StateMachineParserLocalSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo bar = 0\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32\n");
    checkParseError(it, E_SMP_EOF, 2, 4);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 @foo = 0\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 5);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo\n");
    checkParseError(it, E_SMP_EOF, 2, 8);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo @foo\n 0");
    checkParseError(it, E_SMP_LOC_OP, 2, 9);
}

TEST(StateMachineParserLocalSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo > 0\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 9);
}

TEST(StateMachineParserLocalSectionErrors, EofAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo =\n");
    checkParseError(it, E_SMP_EOF, 2, 10);
}

TEST(StateMachineParserLocalSectionErrors, UnexpectedTokenAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = @foo\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 11);
}

TEST(StateMachineParserLocalSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @FOO\n");
    checkParseError(it, E_SMP_ANNOT, 2, 13);
}
