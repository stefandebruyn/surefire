#include "sfa/sup/StateMachineParser.hpp"
#include "sfa/utest/UTest.hpp"

TEST_GROUP(StateMachineParserStateSection)
{
};

TEST(StateMachineParserStateSection, EntryLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = 10\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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

TEST(StateMachineParserStateSection, StepLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".STEP\n"
        "    a = 10\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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

TEST(StateMachineParserStateSection, ExitLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".EXIT\n"
        "    a = 10\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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

TEST(StateMachineParserStateSection, MultipleUnguardedActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a = 1\n"
        "    b = 2\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a = 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
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

TEST(StateMachineParserStateSection, IfAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, IfActionElseAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n"
        "    ELSE: c = 3\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, IfMultipleActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, IfMultipleActionsElseAction)
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
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, IfMultipleActionsElseMultipleActions)
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
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, NestedColonGuards)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b == 2: c = 3\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, NestedBraceGuards)
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
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, ColonGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1: b = 2\n"
        "    c = 3\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, BraceGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        "    a == 1 { b = 2 }\n"
        "    c = 3\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, NewlineAgnosticExceptForGuardsAndActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n\n"
        ".ENTRY\n\n"
        "a == 1\n\n{\n\nb = 2\n}\n\n"
        "c = 3\n\n\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a == 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next != nullptr);

    std::shared_ptr<ExpressionParser::Parse> node = block->guard;
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

TEST(StateMachineParserStateSection, ActionInEveryLabel)
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
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Every label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step != nullptr);
    CHECK_TRUE(parse.exit != nullptr);

    // a = 1
    std::shared_ptr<StateMachineParser::BlockParse> block = parse.entry;
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

TEST(StateMachineParserStateSection, EmptyState)
{
    // Parse state.
    TOKENIZE("[Foo]");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // No labels were parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);
}

TEST(StateMachineParserStateSection, EmptyLabels)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".ENTRY\n"
        ".STEP\n"
        ".EXIT\n");
    StateMachineParser::StateParse parse = {};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // No labels were parsed.
    CHECK_TRUE(parse.entry == nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);
}
