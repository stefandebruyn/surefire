////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/utest/UTestStateMachineParserStateSection.hpp
/// @brief Unit tests for StateMachineParser parsing state sections.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that parsing a state machine generates a certain error.
///
/// @param[in] kIt       State machine config to parse.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error column number.
/// @param[in] kColNum   Expected error line number.
///
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

///
/// @brief Unit tests for StateMachineParser parsing state sections.
///
TEST_GROUP(StateMachineParserStateSection)
{
};

///
/// @test Entry label is parsed correctly.
///
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

///
/// @test Step label is parsed correctly.
///
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

///
/// @test Exit label is parsed correctly.
///
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

///
/// @test A transition is parsed correctly.
///
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

///
/// @test Multiple sequential statements are parsed correctly.
///
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

///
/// @test An if branch is parsed correctly.
///
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

///
/// @test If-else branches are parsed correctly.
///
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

///
/// @test An if branch containing multiple statements is parsed correctly.
///
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

///
/// @test An if branch containing multiple statements and else branch containing
/// a single statement are parsed correctly.
///
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

///
/// @test If-else branches that both contain multiple statements are parsed
/// correctly.
///
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

///
/// @test Nested conditionals using colons are parsed correctly.
///
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

///
/// @test Nested conditionals using braces are parsed correctly.
///
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

///
/// @test A conditional using a colon followed by an unconditional statement is
/// parsed correctly.
///
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

///
/// @test A conditional using braces followed by an unconditional statement is
/// parsed correctly.
///
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

///
/// @test State sections are newline-agnostic except for statements (which must
/// be followed by a newline).
///
TEST(StateMachineParserStateSection, NewlineAgnosticExceptForActions)
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

///
/// @test A state with actions in every label is parsed correctly.
///
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

///
/// @test An empty state is parsed correctly.
///
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

///
/// @test Empty labels are parsed correctly.
///
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

///
/// @test An if-else-if statement is parsed correctly.
///
/// @remark This requires an extra colon after the else, as the state machine
/// DSL has no first-class support for else-if.
///
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

///
/// @test An if-else-if-else statement is parsed correctly.
///
/// @remark This requires braces after the first else, as the state machine DSL
/// has no first-class support for else-if.
///
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

///
/// @brief Unit tests for StateMachineParser parsing state sections with errors.
///
TEST_GROUP(StateMachineParserStateSectionErrors)
{
};

///
/// @test A non-label token where a label is expected generates an error.
///
TEST(StateMachineParserStateSectionErrors, ExpectedLabel)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n"
        "    a = 1\n");
    checkParseError(it, E_SMP_NO_LAB, 2, 1);
}

///
/// @test A colon with no conditional generates an error.
///
TEST(StateMachineParserStateSectionErrors, EmptyGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    : a = 1\n");
    checkParseError(it, E_SMP_GUARD, 3, 5);
}

///
/// @test Errors on conditionals are surfaced.
///
TEST(StateMachineParserStateSectionErrors, SyntaxErrorInGuard)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a == : b = 2\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 7);
}

///
/// @test An unbalanced left brace generates an error.
///
TEST(StateMachineParserStateSectionErrors, UnclosedLeftBrace)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a { b = 2\n");
    checkParseError(it, E_SMP_BRACE, 3, 7);
}

///
/// @test Errors in an if branch are surfaced.
///
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

///
/// @test Errors in an else branch are surfaced.
///
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

///
/// @test Nothing after an else keyword generates an error.
///
TEST(StateMachineParserStateSectionErrors, NothingAfterElse)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a: b = 2\n"
        "    else:\n");
    checkParseError(it, E_SMP_ELSE, 4, 9);
}

///
/// @test Nothing after an element name generates an error.
///
TEST(StateMachineParserStateSectionErrors, NothingAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a\n");
    checkParseError(it, E_SMP_ACT_ELEM, 3, 5);
}

///
/// @test A non-operator token where an operator is expected generates an error.
///
TEST(StateMachineParserStateSectionErrors, UnexpectedTokenAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a @foo 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

///
/// @test An operator other than the assignment operator in an assignment
/// statement generates an error.
///
TEST(StateMachineParserStateSectionErrors, WrongOperatorAfterElementName)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a > 1\n");
    checkParseError(it, E_SMP_ACT_OP, 3, 7);
}

///
/// @test Nothing after an assignment operator generates an error.
///
TEST(StateMachineParserStateSectionErrors, NothingAfterAssignmentOperator)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = \n");
    checkParseError(it, E_SMP_ACT_EXPR, 3, 7);
}

///
/// @test Errors in assignment expressions are surfaced.
///
TEST(StateMachineParserStateSectionErrors, SyntaxErrorInAssignmentAction)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    a = b +\n");
    checkParseError(it, E_EXP_SYNTAX, 3, 11);
}

///
/// @test Nothing after a transition keyword generates an error.
///
TEST(StateMachineParserStateSectionErrors, NothingAfterTransitionKeyword)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    ->\n");
    checkParseError(it, E_SMP_TR_DEST, 3, 5);
}

///
/// @test A non-identifier token after a transition keyword generates an error.
///
TEST(StateMachineParserStateSectionErrors, UnexpectedTokenAfterTransitionOp)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    -> @foo\n");
    checkParseError(it, E_SMP_TR_TOK, 3, 8);
}

///
/// @test An extra token after a valid transition statement generates an error.
///
TEST(StateMachineParserStateSectionErrors, ExtraTokenAfterTransition)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    -> Bar @foo\n");
    checkParseError(it, E_SMP_JUNK, 3, 12);
}

///
/// @test An unexpected token in a label generates an error.
///
TEST(StateMachineParserStateSectionErrors, InvalidFirstActionToken)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        "    @foo\n");
    checkParseError(it, E_SMP_ACT_TOK, 3, 5);
}

///
/// @test Multiple entry labels generates an error.
///
TEST(StateMachineParserStateSectionErrors, MultipleEntryLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".entry\n"
        ".entry\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

///
/// @test Multiple step labels generates an error.
///
TEST(StateMachineParserStateSectionErrors, MultipleStepLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".step\n"
        ".step\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

///
/// @test Multiple exit labels generates an error.
///
TEST(StateMachineParserStateSectionErrors, MultipleExitLabels)
{
    TOKENIZE(
        "[Foo]\n"
        ".exit\n"
        ".exit\n");
    checkParseError(it, E_SMP_LAB_DUPE, 3, 1);
}

///
/// @test An unknown label generates an error.
///
TEST(StateMachineParserStateSectionErrors, UnknownLabel)
{
    TOKENIZE(
        "[Foo]\n"
        ".foo\n");
    checkParseError(it, E_SMP_LAB, 2, 1);
}
