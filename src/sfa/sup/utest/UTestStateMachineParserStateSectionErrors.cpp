#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator &kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    ConfigErrorInfo err;
    StateMachineParser::StateParse parse = {};
    CHECK_ERROR(kRes, StateMachineParser::parseStateSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateMachineParserStateSectionErrors)
{
};

TEST(StateMachineParserStateSectionErrors, UnexpectedTokenInsteadOfLabel)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n"
        "    a = 1\n");
    checkParseError(it, E_SMP_LAB, 2, 1);
}

TEST(StateMachineParserStateSectionErrors, EmptyGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    : a = 1\n");
    checkParseError(it, E_SMP_GUARD, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, SyntaxErrorInGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == : b = 2\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, UnclosedLeftBrace)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a { b = 2\n");
    checkParseError(it, E_SMP_BRACE, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, ErrorInIfBranch)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a {\n"
        "        b == : c = 3\n"
        "    }\n");
    checkParseError(it, E_EXP_SYNTAX, 4, 11);
}

TEST(StateMachineParserStateSectionErrors, ErrorInElseBranch)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a: b = 2\n"
        "    ELSE {\n"
        "        c == : d = 4\n"
        "    }\n");
    checkParseError(it, E_EXP_SYNTAX, 5, 11);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterElse)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a: b = 2\n"
        "    ELSE:\n");
    checkParseError(it, E_SMP_ELSE, 4, 9);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a\n");
    checkParseError(it, E_SMP_ACT_ELEM, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a @foo 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a > 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterAssignmentOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = \n");
    checkParseError(it, E_SMP_ACT_EXPR, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, SyntaxErrorInAssignmentAction)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = b +\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 11);
}

TEST(StateMachineParserStateSectionErrors, TransitionActionWrongOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    > Bar\n");
    checkParseError(it, E_SMP_TR_OP, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterTransitionOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    ->\n");
    checkParseError(it, E_SMP_TR_DEST, 3, 5);
}

TEST(StateMachineParserStateSectionErrors,
     UnexpectedTokenAfterTransitionOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    -> @foo\n");
    checkParseError(it, E_SMP_TR_TOK, 3, 8);
}

TEST(StateMachineParserStateSectionErrors, ExtraTokenAfterTransition)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    -> Bar @foo\n");
    checkParseError(it, E_SMP_TR_JUNK, 3, 12);
}
