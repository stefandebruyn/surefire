#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(const Vec<Token>& kToks,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    StateMachineParser::Parse parse = {};
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParser::parse(kToks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(0, parse.states.size());
    CHECK_TRUE(!parse.hasStateVectorSection);
    CHECK_TRUE(!parse.hasLocalSection);
}

///////////////////////////////// Usage Tests //////////////////////////////////

TEST_GROUP(StateMachineParser)
{
};

TEST(StateMachineParser, AllSections)
{
    // Parse state machine config.
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = 0\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(StateMachineParser::parse(toks, parse, nullptr));

    // Expected number of state vector elements, local elements, and states
    // were parsed.
    CHECK_TRUE(parse.hasStateVectorSection);
    CHECK_TRUE(parse.hasLocalSection);
    CHECK_EQUAL(1, parse.svElems.size());
    CHECK_EQUAL(1, parse.localElems.size());
    CHECK_EQUAL(1, parse.states.size());

    // foo
    CHECK_EQUAL(toks[2], parse.svElems[0].tokType);
    CHECK_EQUAL(toks[3], parse.svElems[0].tokName);
    CHECK_EQUAL("", parse.svElems[0].alias);
    CHECK_TRUE(!parse.svElems[0].readOnly);

    // bar
    CHECK_EQUAL(toks[8], parse.localElems[0].tokType);
    CHECK_EQUAL(toks[9], parse.localElems[0].tokName);
    CHECK_EQUAL(toks[11], parse.localElems[0].tokInitVal);
    CHECK_TRUE(!parse.localElems[0].readOnly);

    // Foo
    CHECK_EQUAL(toks[14], parse.states[0].tokName);
    CHECK_TRUE(parse.states[0].entry != nullptr);
    CHECK_TRUE(parse.states[0].step == nullptr);
    CHECK_TRUE(parse.states[0].exit == nullptr);
    CHECK_TRUE(parse.states[0].entry->guard == nullptr);
    CHECK_TRUE(parse.states[0].entry->action != nullptr);
    CHECK_TRUE(parse.states[0].entry->ifBlock == nullptr);
    CHECK_TRUE(parse.states[0].entry->elseBlock == nullptr);
    CHECK_TRUE(parse.states[0].entry->next == nullptr);

    // foo = 0
    CHECK_EQUAL(toks[18], parse.states[0].entry->action->tokRhs);
    const Ref<const ExpressionParser::Parse> lhs =
        parse.states[0].entry->action->lhs;
    CHECK_TRUE(lhs != nullptr);
    CHECK_EQUAL(toks[20], lhs->data);
    CHECK_TRUE(lhs->left == nullptr);
    CHECK_TRUE(lhs->right == nullptr);
}

TEST(StateMachineParser, EmptySections)
{
    // Parse state machine config.
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "[LOCAL]\n"
        "[Foo]\n");
    StateMachineParser::Parse parse = {};
    CHECK_SUCCESS(StateMachineParser::parse(toks, parse, nullptr));

    // Expected number of state vector elements, local elements, and states
    // were parsed.
    CHECK_TRUE(parse.hasStateVectorSection);
    CHECK_TRUE(parse.hasLocalSection);
    CHECK_EQUAL(0, parse.svElems.size());
    CHECK_EQUAL(0, parse.localElems.size());
    CHECK_EQUAL(1, parse.states.size());

    // Foo
    CHECK_EQUAL(toks[4], parse.states[0].tokName);
    CHECK_TRUE(parse.states[0].entry == nullptr);
    CHECK_TRUE(parse.states[0].step == nullptr);
    CHECK_TRUE(parse.states[0].exit == nullptr);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParserErrors)
{
};

TEST(StateMachineParserErrors, UnexpectedToken)
{
    TOKENIZE(
        "@foo\n"
        "[Foo]\n");
    checkParseError(toks, E_SMP_TOK, 1, 1);
}

TEST(StateMachineParserErrors, ErrorInStateVectorSection)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserErrors, ErrorInLocalSection)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParserErrors, ErrorInStateSection)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_LAB, 2, 1);
}
