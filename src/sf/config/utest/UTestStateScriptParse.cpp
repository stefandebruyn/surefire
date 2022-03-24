#include "sf/config/StateScriptParse.hpp"
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
    CHECK_ERROR(kRes, StateScriptParse::parse(kToks, parse, &err));

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateScriptParse::parse(kToks, parse, nullptr));
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateScriptParse)
{
};

TEST(StateScriptParse, Empty)
{
    TOKENIZE("\n\n\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParse::parse(toks, parse, nullptr));
    CHECK_EQUAL(0, parse->sections.size());
}

TEST(StateScriptParse, EmptySection)
{
    TOKENIZE("[Foo]\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParse::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());
    CHECK_EQUAL(parse->sections[0].tokName, toks[0]);
    CHECK_TRUE(parse->sections[0].block != nullptr);
    CHECK_TRUE(parse->sections[0].block->guard == nullptr);
    CHECK_TRUE(parse->sections[0].block->action == nullptr);
    CHECK_TRUE(parse->sections[0].block->ifBlock == nullptr);
    CHECK_TRUE(parse->sections[0].block->elseBlock == nullptr);
    CHECK_TRUE(parse->sections[0].block->next == nullptr);
    CHECK_TRUE(parse->sections[0].block->assert == nullptr);
}

TEST(StateScriptParse, OneSection)
{
    TOKENIZE(
        "[Foo]\n"
        "foo = 1\n"
        "bar = 2\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParse::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[0]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `foo = 1` action
    CHECK_EQUAL(block->action->tokLhs, toks[2]);
    Ref<const ExpressionParse> node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[4]);
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
    CHECK_EQUAL(block->action->tokLhs, toks[6]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[8]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParse, TwoSections)
{
    TOKENIZE(
        "[Foo]\n"
        "foo = 1\n"
        "bar = 2\n"
        "\n"
        "[Bar]\n"
        "baz = 3\n"
        "qux = 4\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParse::parse(toks, parse, nullptr));
    CHECK_EQUAL(2, parse->sections.size());

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[0]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `foo = 1` action
    CHECK_EQUAL(block->action->tokLhs, toks[2]);
    Ref<const ExpressionParse> node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[4]);
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
    CHECK_EQUAL(block->action->tokLhs, toks[6]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[8]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // `Bar` section
    CHECK_EQUAL(parse->sections[1].tokName, toks[11]);
    CHECK_TRUE(parse->sections[1].block != nullptr);

    // `baz = 3` block
    block = parse->sections[1].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);

    // `baz = 3` action
    CHECK_EQUAL(block->action->tokLhs, toks[13]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[15]);
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
    CHECK_EQUAL(block->action->tokLhs, toks[17]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[19]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParse, Assertion)
{
    TOKENIZE(
        "[Foo]\n"
        "@ASSERT foo == 1\n"
        "bar = 2\n");
    Ref<const StateScriptParse> parse;
    CHECK_SUCCESS(StateScriptParse::parse(toks, parse, nullptr));
    CHECK_EQUAL(1, parse->sections.size());

    // `Foo` section
    CHECK_EQUAL(parse->sections[0].tokName, toks[0]);
    CHECK_TRUE(parse->sections[0].block != nullptr);

    // `@ASSERT foo = 1` block
    Ref<const StateMachineParse::BlockParse> block = parse->sections[0].block;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_TRUE(block->assert != nullptr);

    // `foo == 1` assertion
    Ref<const ExpressionParse> node = block->assert;
    CHECK_EQUAL(node->data, toks[4]);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);
    node = block->assert->left;
    CHECK_EQUAL(node->data, toks[3]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
    node = block->assert->right;
    CHECK_EQUAL(node->data, toks[5]);
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
    CHECK_EQUAL(block->action->tokLhs, toks[7]);
    node = block->action->rhs;
    CHECK_EQUAL(node->data, toks[9]);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);
}

TEST(StateScriptParse, ErrorExpectedSection)
{
    TOKENIZE("foo = 1\n");
    checkParseError(toks, E_SSP_SEC, 1, 1);
}

TEST(StateScriptParse, ErrorInBlock)
{
    TOKENIZE(
        "[Foo]\n"
        "foo = 1 +\n");
    checkParseError(toks, E_EXP_SYNTAX, 2, 9);
}

TEST(StateScriptParse, ErrorInAssertion)
{
    TOKENIZE(
        "[Foo]\n"
        "@ASSERT foo +\n");
    checkParseError(toks, E_EXP_SYNTAX, 2, 13);
}
