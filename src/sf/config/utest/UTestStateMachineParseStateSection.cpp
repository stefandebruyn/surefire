#include "sf/config/StateMachineParse.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator &kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    ErrorInfo err;
    StateMachineParse::StateParse parse = {};
    CHECK_ERROR(kRes, StateMachineParse::parseStateSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);
}

///////////////////////////////// Usage Tests //////////////////////////////////

TEST_GROUP(StateMachineParseStateSection)
{
};

TEST(StateMachineParseStateSection, EntryLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // Entry label contains a single unguarded action.
    CHECK_TRUE(parse.entry->guard == nullptr);
    CHECK_TRUE(parse.entry->ifBlock == nullptr);
    CHECK_TRUE(parse.entry->elseBlock == nullptr);
    CHECK_TRUE(parse.entry->action != nullptr);
    CHECK_TRUE(parse.entry->next == nullptr);

    // a = 10
    CHECK_EQUAL(toks[4], parse.entry->action->tokRhs);
    CHECK_EQUAL(toks[6], parse.entry->action->lhs->data);
    CHECK_TRUE(parse.entry->action->lhs->left == nullptr);
    CHECK_TRUE(parse.entry->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, StepLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".STEP\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step != nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // Entry label contains a single unguarded action.
    CHECK_TRUE(parse.step->guard == nullptr);
    CHECK_TRUE(parse.step->ifBlock == nullptr);
    CHECK_TRUE(parse.step->elseBlock == nullptr);
    CHECK_TRUE(parse.step->action != nullptr);
    CHECK_TRUE(parse.step->next == nullptr);

    // a = 10
    CHECK_EQUAL(toks[4], parse.step->action->tokRhs);
    CHECK_EQUAL(toks[6], parse.step->action->lhs->data);
    CHECK_TRUE(parse.step->action->lhs->left == nullptr);
    CHECK_TRUE(parse.step->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, ExitLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".EXIT\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit != nullptr);

    // Entry label contains a single unguarded action.
    CHECK_TRUE(parse.exit->guard == nullptr);
    CHECK_TRUE(parse.exit->ifBlock == nullptr);
    CHECK_TRUE(parse.exit->elseBlock == nullptr);
    CHECK_TRUE(parse.exit->action != nullptr);
    CHECK_TRUE(parse.exit->next == nullptr);

    // a = 10
    CHECK_EQUAL(toks[4], parse.exit->action->tokRhs);
    CHECK_EQUAL(toks[6], parse.exit->action->lhs->data);
    CHECK_TRUE(parse.exit->action->lhs->left == nullptr);
    CHECK_TRUE(parse.exit->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, MultipleUnguardedActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = 1\n"
        "    b = 2\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a = 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[4], block->action->tokRhs);
    CHECK_EQUAL(toks[6], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // b = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokRhs);
    CHECK_EQUAL(toks[10], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, IfAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokRhs);
    CHECK_EQUAL(toks[10], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, IfActionElseAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n"
        "    ELSE: c = 3\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokRhs);
    CHECK_EQUAL(toks[10], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 3
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[14], block->action->tokRhs);
    CHECK_EQUAL(toks[16], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, IfMultipleActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[9], block->action->tokRhs);
    CHECK_EQUAL(toks[11], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokRhs);
    CHECK_EQUAL(toks[15], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, IfMultipleActionsElseAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n"
        "    ELSE: d = 4\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[9], block->action->tokRhs);
    CHECK_EQUAL(toks[11], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokRhs);
    CHECK_EQUAL(toks[15], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // d = 4
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[21], block->action->tokRhs);
    CHECK_EQUAL(toks[23], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, IfMultipleActionsElseMultipleActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n"
        "    ELSE {\n"
        "        d = 4\n"
        "        e = 5\n"
        "    }\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[9], block->action->tokRhs);
    CHECK_EQUAL(toks[11], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokRhs);
    CHECK_EQUAL(toks[15], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // d = 4
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[22], block->action->tokRhs);
    CHECK_EQUAL(toks[24], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // e = 5
    block = parse.entry->elseBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[26], block->action->tokRhs);
    CHECK_EQUAL(toks[28], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, NestedColonGuards)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b == 2: c = 3\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b == 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    node = block->guard;
    CHECK_EQUAL(toks[9], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[8], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[10], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[12], block->action->tokRhs);
    CHECK_EQUAL(toks[14], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, NestedBraceGuards)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 {\n"
        "        b == 2 {\n"
        "            c = 3\n"
        "        }\n"
        "    }\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b == 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    node = block->guard;
    CHECK_EQUAL(toks[10], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[9], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[11], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[14], block->action->tokRhs);
    CHECK_EQUAL(toks[16], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, ColonGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokRhs);
    CHECK_EQUAL(toks[10], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[12], block->action->tokRhs);
    CHECK_EQUAL(toks[14], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, BraceGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 { b = 2 }\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[4], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokRhs);
    CHECK_EQUAL(toks[10], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokRhs);
    CHECK_EQUAL(toks[15], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, NewlineAgnosticExceptForGuardsAndActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n\n"
        ".ENTRY\n\n"
        "a == 1\n\n{\n\nb = 2\n}\n\n"
        "c = 3\n\n\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[7], node->data);
    CHECK_TRUE(node->left != nullptr);
    CHECK_TRUE(node->right != nullptr);

    node = block->guard->left;
    CHECK_EQUAL(toks[6], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    node = block->guard->right;
    CHECK_EQUAL(toks[8], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 2
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[14], block->action->tokRhs);
    CHECK_EQUAL(toks[16], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[21], block->action->tokRhs);
    CHECK_EQUAL(toks[23], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, ActionInEveryLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = 1\n"
        ".STEP\n"
        "    b = 2\n"
        ".EXIT\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Every label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step != nullptr);
    CHECK_TRUE(parse.exit != nullptr);

    // a = 1
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[4], block->action->tokRhs);
    CHECK_EQUAL(toks[6], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // b = 2
    block = parse.step;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[10], block->action->tokRhs);
    CHECK_EQUAL(toks[12], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);

    // c = 3
    block = parse.exit;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[16], block->action->tokRhs);
    CHECK_EQUAL(toks[18], block->action->lhs->data);
    CHECK_TRUE(block->action->lhs->left == nullptr);
    CHECK_TRUE(block->action->lhs->right == nullptr);
}

TEST(StateMachineParseStateSection, EmptyState)
{
    // Parse state.
    TOKENIZE("[Foo]");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // No labels were parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);
}

TEST(StateMachineParseStateSection, EmptyLabels)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        ".STEP\n"
        ".EXIT\n");
    StateMachineParse::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParse::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // No labels were parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParseStateSectionErrors)
{
};

TEST(StateMachineParseStateSectionErrors, UnexpectedTokenInsteadOfLabel)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n"
        "    a = 1\n");
    checkParseError(it, E_SMP_LAB, 2, 1);
}

TEST(StateMachineParseStateSectionErrors, EmptyGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    : a = 1\n");
    checkParseError(it, E_SMP_GUARD, 3, 5);
}

TEST(StateMachineParseStateSectionErrors, SyntaxErrorInGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == : b = 2\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 7);
}

TEST(StateMachineParseStateSectionErrors, UnclosedLeftBrace)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a { b = 2\n");
    checkParseError(it, E_SMP_BRACE, 3, 7);
}

TEST(StateMachineParseStateSectionErrors, ErrorInIfBranch)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a {\n"
        "        b == : c = 3\n"
        "    }\n");
    checkParseError(it, E_EXP_SYNTAX, 4, 11);
}

TEST(StateMachineParseStateSectionErrors, ErrorInElseBranch)
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

TEST(StateMachineParseStateSectionErrors, NothingAfterElse)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a: b = 2\n"
        "    ELSE:\n");
    checkParseError(it, E_SMP_ELSE, 4, 9);
}

TEST(StateMachineParseStateSectionErrors, NothingAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a\n");
    checkParseError(it, E_SMP_ACT_ELEM, 3, 5);
}

TEST(StateMachineParseStateSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a @foo 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParseStateSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a > 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParseStateSectionErrors, NothingAfterAssignmentOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = \n");
    checkParseError(it, E_SMP_ACT_EXPR, 3, 7);
}

TEST(StateMachineParseStateSectionErrors, SyntaxErrorInAssignmentAction)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = b +\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 11);
}

TEST(StateMachineParseStateSectionErrors, TransitionActionWrongOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    > Bar\n");
    checkParseError(it, E_SMP_TR_OP, 3, 5);
}

TEST(StateMachineParseStateSectionErrors, NothingAfterTransitionOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    ->\n");
    checkParseError(it, E_SMP_TR_DEST, 3, 5);
}

TEST(StateMachineParseStateSectionErrors,
     UnexpectedTokenAfterTransitionOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    -> @foo\n");
    checkParseError(it, E_SMP_TR_TOK, 3, 8);
}

TEST(StateMachineParseStateSectionErrors, ExtraTokenAfterTransition)
{
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    -> Bar @foo\n");
    checkParseError(it, E_SMP_TR_JUNK, 3, 12);
}
