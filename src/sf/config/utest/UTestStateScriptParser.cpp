#include "sf/config/StateScriptParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(const Vec<Token>& kToks,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    Ref<const StateScriptParse> parse;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateScriptParser::parse(kToks, parse, &err));

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateScriptParser::parse(kToks, parse, nullptr));
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateScriptParser)
{
};

TEST(StateScriptParser, NoStateSections)
{
    TOKENIZE(
        "\n\n\n"
        "[options]\n"
        "delta_t 1\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(0, parse->sections.size());
    CHECK_EQUAL(toks[6], parse->config.tokDeltaT);
}

TEST(StateScriptParser, ConfigInitStateOption)
{
    TOKENIZE(
        "[options]\n"
        "init_state foo\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(0, parse->sections.size());
    CHECK_EQUAL(toks[3], parse->config.tokInitState);
}

TEST(StateScriptParser, EmptyStateSection)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());
    CHECK_EQUAL(parse->sections[0].tokName, toks[6]);
    CHECK_TRUE(parse->sections[0].block != nullptr);
    CHECK_TRUE(parse->sections[0].block->guard == nullptr);
    CHECK_TRUE(parse->sections[0].block->action == nullptr);
    CHECK_TRUE(parse->sections[0].block->ifBlock == nullptr);
    CHECK_TRUE(parse->sections[0].block->elseBlock == nullptr);
    CHECK_TRUE(parse->sections[0].block->next == nullptr);
    CHECK_TRUE(parse->sections[0].block->assert == nullptr);
    CHECK_EQUAL(toks[3], parse->config.tokDeltaT);
}

TEST(StateScriptParser, OneSection)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1\n"
        "bar = 2\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());
    CHECK_EQUAL(toks[3], parse->config.tokDeltaT);

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[6]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `foo = 1` action
    CHECK_EQUAL(block->action->tokLhs, toks[8]);
    Ref<const ExpressionParse> node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `bar = 2` block
    block = block->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next == nullptr);

    // `bar = 2` action
    CHECK_EQUAL(block->action->tokLhs, toks[12]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[14]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParser, TwoSections)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1\n"
        "bar = 2\n"
        "\n"
        "[Bar]\n"
        "baz = 3\n"
        "qux = 4\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(2, parse->sections.size());
    CHECK_EQUAL(toks[3], parse->config.tokDeltaT);

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[6]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `foo = 1` action
    CHECK_EQUAL(block->action->tokLhs, toks[8]);
    Ref<const ExpressionParse> node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `bar = 2` block
    block = block->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next == nullptr);

    // `bar = 2` action
    CHECK_EQUAL(block->action->tokLhs, toks[12]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[14]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `Bar` section
    CHECK_EQUAL(parse->sections[1].tokName, toks[17]);
    CHECK_TRUE(parse->sections[1].block != nullptr);

    // `baz = 3` block
    block = parse->sections[1].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `baz = 3` action
    CHECK_EQUAL(block->action->tokLhs, toks[19]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[21]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `qux = 4` block
    block = block->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next == nullptr);

    // `qux = 4` action
    CHECK_EQUAL(block->action->tokLhs, toks[23]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[25]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParser, Assertion)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@assert foo == 1\n"
        "bar = 2\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParser::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());
    CHECK_EQUAL(toks[3], parse->config.tokDeltaT);

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[6]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `@assert foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_TRUE(block->assert != nullptr);

    // `foo == 1` assertion
    Ref<const ExpressionParse> node = block->assert;
    CHECK_EQUAL(node->data, toks[10]);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);
    node = block->assert->left;
    CHECK_EQUAL(node->data, toks[9]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    node = block->assert->right;
    CHECK_EQUAL(node->data, toks[11]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `bar = 2` block
    block = block->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_TRUE(block->assert == nullptr);

    // `bar = 2` action
    CHECK_EQUAL(block->action->tokLhs, toks[13]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[15]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParser, ErrorExpectedSection)
{
    TOKENIZE("foo = 1\n");
    checkParseError(toks, E_SSP_SEC, 1, 1);
}

TEST(StateScriptParser, ErrorInBlock)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1 +\n");
    checkParseError(toks, E_EXP_SYNTAX, 5, 9);
}

TEST(StateScriptParser, ErrorInAssertion)
{
    TOKENIZE(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@assert foo +\n");
    checkParseError(toks, E_EXP_SYNTAX, 5, 13);
}

TEST(StateScriptParser, ErrorUnexpectedTokenAfterDeltaT)
{
    TOKENIZE(
        "[options]\n"
        "delta_t foo\n");
    checkParseError(toks, E_SSP_DT, 2, 1);
}

TEST(StateScriptParser, ErrorEofAfterDeltaT)
{
    TOKENIZE(
        "[options]\n"
        "delta_t\n");
    checkParseError(toks, E_SSP_DT, 2, 1);
}

TEST(StateScriptParser, ErrorUnexpectedTokenAfterInitState)
{
    TOKENIZE(
        "[options]\n"
        "init_state @foo\n");
    checkParseError(toks, E_SSP_STATE, 2, 1);
}

TEST(StateScriptParser, ErrorEofAfterInitState)
{
    TOKENIZE(
        "[options]\n"
        "init_state\n");
    checkParseError(toks, E_SSP_STATE, 2, 1);
}

TEST(StateScriptParser, ErrorUnknownConfigOption)
{
    TOKENIZE(
        "[options]\n"
        "foo 3\n");
    checkParseError(toks, E_SSP_CONFIG, 2, 1);
}

TEST(StateScriptParser, ErrorExtraTokenAfterStop)
{
    TOKENIZE(
        "[all_states]\n"
        "true: @stop foo\n");
    checkParseError(toks, E_SMP_JUNK, 2, 13);
}
