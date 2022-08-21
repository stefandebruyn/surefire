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
/// @file  sf/config/utest/UTestStateScriptParser.hpp
/// @brief Unit tests for StateScriptParser.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateScriptParser.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that parsing a state script generates a certain error.
///
/// @param[in] kToks     State script config to parse.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error column number.
/// @param[in] kColNum   Expected error line number.
///
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

///
/// @brief Unit tests for StateScriptParser.
///
TEST_GROUP(StateScriptParser)
{
};

///
/// @test State script with no state sections is parsed correctly.
///
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

///
/// @test Initial state option is parsed correctly.
///
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

///
/// @test Empty state section is parsed correctly.
///
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

///
/// @test State script with a single state section is parsed correctly.
///
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

///
/// @test State script with two state sections is parsed correctly.
///
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

///
/// @test Assertions are parsed correctly.
///
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

///
/// @test A statement outside of a section generates an error.
///
TEST(StateScriptParser, ErrorExpectedSection)
{
    TOKENIZE("foo = 1\n");
    checkParseError(toks, E_SSP_SEC, 1, 1);
}

///
/// @test Errors in parsing a code block are surfaced.
///
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

///
/// @test Errors in assertion expressions are surfaced.
///
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

///
/// @test A non-constant token after the delta T option generates an error.
///
TEST(StateScriptParser, ErrorUnexpectedTokenAfterDeltaT)
{
    TOKENIZE(
        "[options]\n"
        "delta_t foo\n");
    checkParseError(toks, E_SSP_DT, 2, 1);
}

///
/// @test No tokens after the delta T option generates an error.
///
TEST(StateScriptParser, ErrorEofAfterDeltaT)
{
    TOKENIZE(
        "[options]\n"
        "delta_t\n");
    checkParseError(toks, E_SSP_DT, 2, 1);
}

///
/// @test A non-identifier token after the delta T option generates an error.
///
TEST(StateScriptParser, ErrorUnexpectedTokenAfterInitState)
{
    TOKENIZE(
        "[options]\n"
        "init_state @foo\n");
    checkParseError(toks, E_SSP_STATE, 2, 1);
}

///
/// @test No tokens after the initial state option generates an error.
///
TEST(StateScriptParser, ErrorEofAfterInitState)
{
    TOKENIZE(
        "[options]\n"
        "init_state\n");
    checkParseError(toks, E_SSP_STATE, 2, 1);
}

///
/// @test An unknown option generates an error.
///
TEST(StateScriptParser, ErrorUnknownOption)
{
    TOKENIZE(
        "[options]\n"
        "foo 3\n");
    checkParseError(toks, E_SSP_CONFIG, 2, 1);
}

///
/// @test Extraneous tokens after a stop annotation generate an error.
///
TEST(StateScriptParser, ErrorExtraTokenAfterStop)
{
    TOKENIZE(
        "[all_states]\n"
        "true: @stop foo\n");
    checkParseError(toks, E_SMP_JUNK, 2, 13);
}
