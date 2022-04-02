#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static void checkParseError(TokenIterator& kIt,
                            const Result kRes,
                            const I32 kLineNum,
                            const I32 kColNum)
{
    // Got expected return code from parser.
    ErrorInfo err;
    StateMachineParse::StateParse parse{};
    TokenIterator itCpy = kIt;
    CHECK_ERROR(kRes, StateMachineParser::parseStateSection(kIt, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes,
                StateMachineParser::parseStateSection(itCpy, parse, nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateMachineParserStateSection)
{
};

TEST(StateMachineParserStateSection, EntryLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse{};
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
    CHECK_EQUAL(toks[4], parse.entry->action->tokLhs);
    CHECK_EQUAL(toks[6], parse.entry->action->rhs->data);
    CHECK_TRUE(parse.entry->action->rhs->left == nullptr);
    CHECK_TRUE(parse.entry->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, StepLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".step\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse{};
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
    CHECK_EQUAL(toks[4], parse.step->action->tokLhs);
    CHECK_EQUAL(toks[6], parse.step->action->rhs->data);
    CHECK_TRUE(parse.step->action->rhs->left == nullptr);
    CHECK_TRUE(parse.step->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, ExitLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".exit\n"
        "    a = 10\n");
    StateMachineParse::StateParse parse{};
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
    CHECK_EQUAL(toks[4], parse.exit->action->tokLhs);
    CHECK_EQUAL(toks[6], parse.exit->action->rhs->data);
    CHECK_TRUE(parse.exit->action->rhs->left == nullptr);
    CHECK_TRUE(parse.exit->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, Transition)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    -> Bar\n");
    StateMachineParse::StateParse parse{};
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

    // -> Bar
    CHECK_TRUE(parse.entry->action->rhs == nullptr);
    CHECK_EQUAL(toks[5], parse.entry->action->tokDestState);
    CHECK_EQUAL(toks[4], parse.entry->action->tokTransitionKeyword);
}

TEST(StateMachineParserStateSection, MultipleUnguardedActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = 1\n"
        "    b = 2\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[4], block->action->tokLhs);
    CHECK_EQUAL(toks[6], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // b = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[8], block->action->tokLhs);
    CHECK_EQUAL(toks[10], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1: b = 2\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[8], block->action->tokLhs);
    CHECK_EQUAL(toks[10], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfActionElseAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1: b = 2\n"
        "    else: c = 3\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[8], block->action->tokLhs);
    CHECK_EQUAL(toks[10], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 3
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[14], block->action->tokLhs);
    CHECK_EQUAL(toks[16], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfMultipleActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[9], block->action->tokLhs);
    CHECK_EQUAL(toks[11], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokLhs);
    CHECK_EQUAL(toks[15], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfMultipleActionsElseAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n"
        "    else: d = 4\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[9], block->action->tokLhs);
    CHECK_EQUAL(toks[11], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokLhs);
    CHECK_EQUAL(toks[15], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // d = 4
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[21], block->action->tokLhs);
    CHECK_EQUAL(toks[23], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfMultipleActionsElseMultipleActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1 {\n"
        "        b = 2\n"
        "        c = 3\n"
        "    }\n"
        "    else {\n"
        "        d = 4\n"
        "        e = 5\n"
        "    }\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[9], block->action->tokLhs);
    CHECK_EQUAL(toks[11], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 3
    block = parse.entry->ifBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokLhs);
    CHECK_EQUAL(toks[15], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // d = 4
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next != nullptr);
    CHECK_EQUAL(toks[22], block->action->tokLhs);
    CHECK_EQUAL(toks[24], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // e = 5
    block = parse.entry->elseBlock->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[26], block->action->tokLhs);
    CHECK_EQUAL(toks[28], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, NestedColonGuards)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1: b == 2: c = 3\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[12], block->action->tokLhs);
    CHECK_EQUAL(toks[14], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, NestedBraceGuards)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1 {\n"
        "        b == 2 {\n"
        "            c = 3\n"
        "        }\n"
        "    }\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[14], block->action->tokLhs);
    CHECK_EQUAL(toks[16], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, ColonGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1: b = 2\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[8], block->action->tokLhs);
    CHECK_EQUAL(toks[10], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[12], block->action->tokLhs);
    CHECK_EQUAL(toks[14], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, BraceGuardFollowedByAction)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == 1 { b = 2 }\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[8], block->action->tokLhs);
    CHECK_EQUAL(toks[10], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[13], block->action->tokLhs);
    CHECK_EQUAL(toks[15], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, NewlineAgnosticExceptForGuardsAndActions)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n\n"
        ".entry\n\n"
        "a == 1\n\n{\n\nb = 2\n}\n\n"
        "c = 3\n\n\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[14], block->action->tokLhs);
    CHECK_EQUAL(toks[16], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 2
    block = parse.entry->next;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[21], block->action->tokLhs);
    CHECK_EQUAL(toks[23], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, ActionInEveryLabel)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = 1\n"
        ".step\n"
        "    b = 2\n"
        ".exit\n"
        "    c = 3\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
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
    CHECK_EQUAL(toks[4], block->action->tokLhs);
    CHECK_EQUAL(toks[6], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // b = 2
    block = parse.step;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[10], block->action->tokLhs);
    CHECK_EQUAL(toks[12], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c = 3
    block = parse.exit;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[16], block->action->tokLhs);
    CHECK_EQUAL(toks[18], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, EmptyState)
{
    // Parse state.
    TOKENIZE("[Foo]");
    StateMachineParse::StateParse parse{};
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
        ".entry\n"
        ".step\n"
        ".exit\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Empty labels were parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.entry->guard == nullptr);
    CHECK_TRUE(parse.entry->action == nullptr);
    CHECK_TRUE(parse.entry->ifBlock == nullptr);
    CHECK_TRUE(parse.entry->elseBlock == nullptr);
    CHECK_TRUE(parse.entry->next == nullptr);
    CHECK_TRUE(parse.entry->assert == nullptr);

    CHECK_TRUE(parse.step != nullptr);
    CHECK_TRUE(parse.step->guard == nullptr);
    CHECK_TRUE(parse.step->action == nullptr);
    CHECK_TRUE(parse.step->ifBlock == nullptr);
    CHECK_TRUE(parse.step->elseBlock == nullptr);
    CHECK_TRUE(parse.step->next == nullptr);
    CHECK_TRUE(parse.step->assert == nullptr);

    CHECK_TRUE(parse.exit != nullptr);
    CHECK_TRUE(parse.exit->guard == nullptr);
    CHECK_TRUE(parse.exit->action == nullptr);
    CHECK_TRUE(parse.exit->ifBlock == nullptr);
    CHECK_TRUE(parse.exit->elseBlock == nullptr);
    CHECK_TRUE(parse.exit->next == nullptr);
    CHECK_TRUE(parse.exit->assert == nullptr);
}

TEST(StateMachineParserStateSection, IfElseIf)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    if a: b = 1\n"
        "    else: if c: d = 2\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 1
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[7], block->action->tokLhs);
    CHECK_EQUAL(toks[9], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    node = block->guard;
    CHECK_EQUAL(toks[14], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // d = 2
    block = parse.entry->elseBlock->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[16], block->action->tokLhs);
    CHECK_EQUAL(toks[18], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

TEST(StateMachineParserStateSection, IfElseIfElse)
{
    // Parse state.
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    if a: b = 1\n"
        "    else { if c: d = 2\n"
        "           else: e = 3 }\n");
    StateMachineParse::StateParse parse{};
    CHECK_SUCCESS(StateMachineParser::parseStateSection(it, parse, nullptr));
    CHECK_TRUE(it.eof());

    // State name was parsed correctly.
    CHECK_EQUAL(toks[0], parse.tokName);

    // Only an entry label was parsed.
    CHECK_TRUE(parse.entry != nullptr);
    CHECK_TRUE(parse.step == nullptr);
    CHECK_TRUE(parse.exit == nullptr);

    // a
    Ref<const StateMachineParse::BlockParse> block = parse.entry;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    Ref<const ExpressionParse> node = block->guard;
    CHECK_EQUAL(toks[5], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // b = 1
    block = parse.entry->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[7], block->action->tokLhs);
    CHECK_EQUAL(toks[9], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // c
    block = parse.entry->elseBlock;
    CHECK_TRUE(block->guard != nullptr);
    CHECK_TRUE(block->ifBlock != nullptr);
    CHECK_TRUE(block->elseBlock != nullptr);
    CHECK_TRUE(block->action == nullptr);
    CHECK_TRUE(block->next == nullptr);

    node = block->guard;
    CHECK_EQUAL(toks[14], node->data);
    CHECK_TRUE(node->left == nullptr);
    CHECK_TRUE(node->right == nullptr);

    // d = 2
    block = parse.entry->elseBlock->ifBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[16], block->action->tokLhs);
    CHECK_EQUAL(toks[18], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);

    // e = 3
    block = parse.entry->elseBlock->elseBlock;
    CHECK_TRUE(block->guard == nullptr);
    CHECK_TRUE(block->ifBlock == nullptr);
    CHECK_TRUE(block->elseBlock == nullptr);
    CHECK_TRUE(block->action != nullptr);
    CHECK_TRUE(block->next == nullptr);
    CHECK_EQUAL(toks[22], block->action->tokLhs);
    CHECK_EQUAL(toks[24], block->action->rhs->data);
    CHECK_TRUE(block->action->rhs->left == nullptr);
    CHECK_TRUE(block->action->rhs->right == nullptr);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineParserStateSectionErrors)
{
};

TEST(StateMachineParserStateSectionErrors, ExpectedLabel)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n"
        "    a = 1\n");
    checkParseError(it, E_SMP_NO_LAB, 2, 1);
}

TEST(StateMachineParserStateSectionErrors, EmptyGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    : a = 1\n");
    checkParseError(it, E_SMP_GUARD, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, SyntaxErrorInGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == : b = 2\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, UnclosedLeftBrace)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a { b = 2\n");
    checkParseError(it, E_SMP_BRACE, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, ErrorInIfBranch)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a {\n"
        "        b == : c = 3\n"
        "    }\n");
    checkParseError(it, E_EXP_SYNTAX, 4, 11);
}

TEST(StateMachineParserStateSectionErrors, ErrorInElseBranch)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a: b = 2\n"
        "    else {\n"
        "        c == : d = 4\n"
        "    }\n");
    checkParseError(it, E_EXP_SYNTAX, 5, 11);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterElse)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a: b = 2\n"
        "    else:\n");
    checkParseError(it, E_SMP_ELSE, 4, 9);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a\n");
    checkParseError(it, E_SMP_ACT_ELEM, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a @foo 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a > 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterAssignmentOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = \n");
    checkParseError(it, E_SMP_ACT_EXPR, 3, 7);
}

TEST(StateMachineParserStateSectionErrors, SyntaxErrorInAssignmentAction)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = b +\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 11);
}

TEST(StateMachineParserStateSectionErrors, NothingAfterTransitionOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    ->\n");
    checkParseError(it, E_SMP_TR_DEST, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, UnexpectedTokenAfterTransitionOp)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    -> @foo\n");
    checkParseError(it, E_SMP_TR_TOK, 3, 8);
}

TEST(StateMachineParserStateSectionErrors, ExtraTokenAfterTransition)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    -> Bar @foo\n");
    checkParseError(it, E_SMP_JUNK, 3, 12);
}

TEST(StateMachineParserStateSectionErrors, InvalidFirstActionToken)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    @foo\n");
    checkParseError(it, E_SMP_ACT_TOK, 3, 5);
}

TEST(StateMachineParserStateSectionErrors, MultipleEntryLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        ".entry\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

TEST(StateMachineParserStateSectionErrors, MultipleStepLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".step\n"
        ".step\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

TEST(StateMachineParserStateSectionErrors, MultipleExitLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".exit\n"
        ".exit\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

TEST(StateMachineParserStateSectionErrors, UnknownLabel)
{
    TOKENIZE(
        "[Foo]\n"
        ".foo\n");
    checkParseError(it, E_SMP_LAB, 2, 1);
}
