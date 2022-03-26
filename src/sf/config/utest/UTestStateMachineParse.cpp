#include "sf/config/StateMachineParse.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(const Vec<Token>& kToks,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    Ref<const StateMachineParse> parse;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParse::parse(kToks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineParse::parse(kToks, parse, nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateMachineParse)
{
};

TEST(StateMachineParse, AllSections)
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
    Ref<const StateMachineParse> parse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, parse, nullptr));

    // Expected number of state vector elements, local elements, and states
    // were parsed.
    CHECK_TRUE(parse->hasStateVectorSection);
    CHECK_TRUE(parse->hasLocalSection);
    CHECK_EQUAL(1, parse->svElems.size());
    CHECK_EQUAL(1, parse->localElems.size());
    CHECK_EQUAL(1, parse->states.size());

    // foo
    CHECK_EQUAL(toks[2], parse->svElems[0].tokType);
    CHECK_EQUAL(toks[3], parse->svElems[0].tokName);
    CHECK_TRUE(!parse->svElems[0].readOnly);

    // bar
    CHECK_EQUAL(toks[8], parse->localElems[0].tokType);
    CHECK_EQUAL(toks[9], parse->localElems[0].tokName);
    // CHECK_EQUAL(toks[11], parse->localElems[0].tokInitVal);
    CHECK_TRUE(!parse->localElems[0].readOnly);

    // Foo
    CHECK_EQUAL(toks[14], parse->states[0].tokName);
    CHECK_TRUE(parse->states[0].entry != nullptr);
    CHECK_TRUE(parse->states[0].step == nullptr);
    CHECK_TRUE(parse->states[0].exit == nullptr);
    CHECK_TRUE(parse->states[0].entry->guard == nullptr);
    CHECK_TRUE(parse->states[0].entry->action != nullptr);
    CHECK_TRUE(parse->states[0].entry->ifBlock == nullptr);
    CHECK_TRUE(parse->states[0].entry->elseBlock == nullptr);
    CHECK_TRUE(parse->states[0].entry->next == nullptr);

    // foo = 0
    CHECK_EQUAL(toks[18], parse->states[0].entry->action->tokLhs);
    const Ref<const ExpressionParse> rhs = parse->states[0].entry->action->rhs;
    CHECK_TRUE(rhs != nullptr);
    CHECK_EQUAL(toks[20], rhs->data);
    CHECK_TRUE(rhs->left == nullptr);
    CHECK_TRUE(rhs->right == nullptr);
}

TEST(StateMachineParse, EmptySections)
{
    // Parse state machine config.
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "[LOCAL]\n"
        "[Foo]\n");
    Ref<const StateMachineParse> parse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, parse, nullptr));

    // Expected number of state vector elements, local elements, and states
    // were parsed.
    CHECK_TRUE(parse->hasStateVectorSection);
    CHECK_TRUE(parse->hasLocalSection);
    CHECK_EQUAL(0, parse->svElems.size());
    CHECK_EQUAL(0, parse->localElems.size());
    CHECK_EQUAL(1, parse->states.size());

    // Foo
    CHECK_EQUAL(toks[4], parse->states[0].tokName);
    CHECK_TRUE(parse->states[0].entry == nullptr);
    CHECK_TRUE(parse->states[0].step == nullptr);
    CHECK_TRUE(parse->states[0].exit == nullptr);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParseErrors)
{
};

TEST(StateMachineParseErrors, UnexpectedToken)
{
    TOKENIZE(
        "@foo\n"
        "[Foo]\n");
    checkParseError(toks, E_SMP_TOK, 1, 1);
}

TEST(StateMachineParseErrors, ErrorInStateVectorSection)
{
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParseErrors, ErrorInLocalSection)
{
    TOKENIZE(
        "[LOCAL]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

TEST(StateMachineParseErrors, ErrorInStateSection)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_NO_LAB, 2, 1);
}
