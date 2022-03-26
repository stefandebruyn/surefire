#include "sf/config/StateMachineParse.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator& kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    Vec<StateMachineParse::LocalElementParse> parse;
    ErrorInfo err;
    TokenIterator itCpy = kIt;
    CHECK_ERROR(kRes, StateMachineParse::parseLocalSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineParse::parseLocalSection(itCpy,
                                                           parse,
                                                           nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateMachineParseLocalSection)
{
};

TEST(StateMachineParseLocalSection, Empty)
{
    // Parse local section.
    TOKENIZE("[LOCAL]");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParseLocalSection, EmptyWithNewlines)
{
    TOKENIZE("[LOCAL]\n\n\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));
    CHECK_EQUAL(0, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());
}

TEST(StateMachineParseLocalSection, OneElement)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

TEST(StateMachineParseLocalSection, ReadOnlyAnnotation)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[0].readOnly);
}

TEST(StateMachineParseLocalSection, MultipleElements)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0\n"
        "BOOL baz = FALSE\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 10
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // F64 bar = 0.0
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // BOOL baz = FALSE
    CHECK_TRUE(parse[2].tokType == toks[12]);
    CHECK_TRUE(parse[2].tokName == toks[13]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[15]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

TEST(StateMachineParseLocalSection, MultipleElementsWithAnnotations)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 10\n"
        "F64 bar = 0.0 @READ_ONLY\n"
        "BOOL baz = FALSE @READ_ONLY\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(3, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = 10
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // F64 bar = 0.0 @READ_ONLY
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[1].readOnly);

    // BOOL baz = FALSE @READ_ONLY
    CHECK_TRUE(parse[2].tokType == toks[13]);
    CHECK_TRUE(parse[2].tokName == toks[14]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[16]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(parse[2].readOnly);
}

TEST(StateMachineParseLocalSection, AllElementTypes)
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
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(11, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I8 a = 0
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);

    // I16 b = 0
    CHECK_TRUE(parse[1].tokType == toks[7]);
    CHECK_TRUE(parse[1].tokName == toks[8]);
    node = parse[1].initValExpr;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[1].readOnly);

    // I32 c = 0
    CHECK_TRUE(parse[2].tokType == toks[12]);
    CHECK_TRUE(parse[2].tokName == toks[13]);
    node = parse[2].initValExpr;
    CHECK_EQUAL(node->data, toks[15]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[2].readOnly);

    // I64 d = 0
    CHECK_TRUE(parse[3].tokType == toks[17]);
    CHECK_TRUE(parse[3].tokName == toks[18]);
    node = parse[3].initValExpr;
    CHECK_EQUAL(node->data, toks[20]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[3].readOnly);

    // U8 e = 0
    CHECK_TRUE(parse[4].tokType == toks[22]);
    CHECK_TRUE(parse[4].tokName == toks[23]);
    node = parse[4].initValExpr;
    CHECK_EQUAL(node->data, toks[25]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[4].readOnly);

    // U16 f = 0
    CHECK_TRUE(parse[5].tokType == toks[27]);
    CHECK_TRUE(parse[5].tokName == toks[28]);
    node = parse[5].initValExpr;
    CHECK_EQUAL(node->data, toks[30]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[5].readOnly);

    // U32 g = 0
    CHECK_TRUE(parse[6].tokType == toks[32]);
    CHECK_TRUE(parse[6].tokName == toks[33]);
    node = parse[6].initValExpr;
    CHECK_EQUAL(node->data, toks[35]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[6].readOnly);

    // U64 h = 0
    CHECK_TRUE(parse[7].tokType == toks[37]);
    CHECK_TRUE(parse[7].tokName == toks[38]);
    node = parse[7].initValExpr;
    CHECK_EQUAL(node->data, toks[40]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[7].readOnly);

    // F32 i = 0
    CHECK_TRUE(parse[8].tokType == toks[42]);
    CHECK_TRUE(parse[8].tokName == toks[43]);
    node = parse[8].initValExpr;
    CHECK_EQUAL(node->data, toks[45]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[8].readOnly);

    // F64 j = 0
    CHECK_TRUE(parse[9].tokType == toks[47]);
    CHECK_TRUE(parse[9].tokName == toks[48]);
    node = parse[9].initValExpr;
    CHECK_EQUAL(node->data, toks[50]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[9].readOnly);

    // BOOL k = FALSE
    CHECK_TRUE(parse[10].tokType == toks[52]);
    CHECK_TRUE(parse[10].tokName == toks[53]);
    node = parse[10].initValExpr;
    CHECK_EQUAL(node->data, toks[55]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[10].readOnly);
}

TEST(StateMachineParseLocalSection, MultipleTermsInElementAssignment)
{
    // Parse local section.
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = a + b\n");
    Vec<StateMachineParse::LocalElementParse> parse;
    CHECK_SUCCESS(StateMachineParse::parseLocalSection(it, parse, nullptr));

    // Parsed expected number of elements. Iterator scanned through the entire
    // section.
    CHECK_EQUAL(1, parse.size());
    CHECK_EQUAL(toks.size(), it.idx());

    // I32 foo = a + b
    CHECK_TRUE(parse[0].tokType == toks[2]);
    CHECK_TRUE(parse[0].tokName == toks[3]);
    Ref<const ExpressionParse> node = parse[0].initValExpr;
    CHECK_EQUAL(node->data, toks[6]);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);
    node = parse[0].initValExpr->left;
    CHECK_EQUAL(node->data, toks[5]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    node = parse[0].initValExpr->right;
    CHECK_EQUAL(node->data, toks[7]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    CHECK_TRUE(!parse[0].readOnly);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParseLocalSectionErrors)
{
};

TEST(StateMachineParseLocalSectionErrors, RedundantReadOnlyAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY @READ_ONLY\n");
    checkParseError(it, E_SMP_RO_MULT, 2, 24);
}

TEST(StateMachineParseLocalSectionErrors, ExpectedElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo bar = 0\n");
    checkParseError(it, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParseLocalSectionErrors, EofAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

TEST(StateMachineParseLocalSectionErrors, UnexpectedTokenAfterElementType)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 @foo = 0\n");
    checkParseError(it, E_SMP_ELEM_NAME, 2, 1);
}

TEST(StateMachineParseLocalSectionErrors, EofAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

TEST(StateMachineParseLocalSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo @foo\n 0");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

TEST(StateMachineParseLocalSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo > 0\n");
    checkParseError(it, E_SMP_LOC_OP, 2, 5);
}

TEST(StateMachineParseLocalSectionErrors, EofAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo =\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 9);
}

TEST(StateMachineParseLocalSectionErrors, UnexpectedTokenAfterAssignmentOp)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = @foo\n");
    checkParseError(it, E_SMP_LOC_VAL, 2, 9);
}

TEST(StateMachineParseLocalSectionErrors, UnknownAnnotation)
{
    TOKENIZE(
        "[LOCAL]\n"
        "I32 foo = 0 @FOO\n");
    checkParseError(it, E_SMP_ANNOT, 2, 13);
}
