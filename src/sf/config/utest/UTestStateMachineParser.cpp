////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/config/utest/UTestStateMachineParser.hpp
/// @brief Unit tests for StateMachineParser.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateMachineParser.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that parsing a state machine generates a certain error.
///
/// @param[in] kToks     State machine config to parse.
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
    Ref<const StateMachineParse> parse;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineParser::parse(kToks, parse, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Parse was not populated.
    CHECK_TRUE(parse == nullptr);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineParser::parse(kToks, parse, nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for StateMachineParser.
///
/// @remark This test group is relatively small because parsing of the different
/// state machine sections is tested in section-specific test groups. The tests
/// in this group mostly check that the results of section parsing are "bubbled
/// up" to the final parse.
///
TEST_GROUP(StateMachineParser)
{
};

///
/// @test A state machine with state vector, local, and state sections is
/// correctly parsed
///
TEST(StateMachineParser, AllSections)
{
    // Parse state machine config.
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 0\n");
    Ref<const StateMachineParse> parse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, parse, nullptr));

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

///
/// @test A state machine with empty state vector, local, and state sections is
/// correctly parsed
///
TEST(StateMachineParser, EmptySections)
{
    // Parse state machine config.
    TOKENIZE(
        "[state_vector]\n"
        "[local]\n"
        "[Foo]\n");
    Ref<const StateMachineParse> parse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, parse, nullptr));

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

///
/// @brief Unit tests for StateMachineParser errors.
///
/// @remark This test group is relatively small because parsing of the different
/// state machine sections is tested in section-specific test groups. The tests
/// in this group mostly check that the results of section parsing are "bubbled
/// up" to the final parse.
///
TEST_GROUP(StateMachineParserErrors)
{
};

///
/// @test An unexpected token outside a section generates an error.
///
TEST(StateMachineParserErrors, UnexpectedToken)
{
    TOKENIZE(
        "@foo\n"
        "[Foo]\n");
    checkParseError(toks, E_SMP_TOK, 1, 1);
}

///
/// @test Errors in the state vector section are surfaced.
///
TEST(StateMachineParserErrors, ErrorInStateVectorSection)
{
    TOKENIZE(
        "[state_vector]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

///
/// @test Errors in the local section are surfaced.
///
TEST(StateMachineParserErrors, ErrorInLocalSection)
{
    TOKENIZE(
        "[local]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_ELEM_TYPE, 2, 1);
}

///
/// @test Errors in a state section are surfaced.
///
TEST(StateMachineParserErrors, ErrorInStateSection)
{
    TOKENIZE(
        "[Foo]\n"
        "@foo\n");
    checkParseError(toks, E_SMP_NO_LAB, 2, 1);
}
