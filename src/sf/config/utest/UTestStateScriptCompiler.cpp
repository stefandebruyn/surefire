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
/// @file  sf/config/utest/UTestStateScriptCompiler.hpp
/// @brief Unit tests for StateScriptCompiler.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/StateScriptCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Initializes the state vector.
///
/// @param[in] kSrc  State vector config as string.
///
#define INIT_SV(kSrc)                                                          \
    /* Compile state vector. */                                                \
    std::stringstream svSrc(kSrc);                                             \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(svSrc, svAsm, nullptr));        \
                                                                               \
    /* Get state vector. */                                                    \
    StateVector& sv = svAsm->get();

///
/// @brief Initializes the state machine.
///
/// @param[in] kSrc  State machine config as string.
///
#define INIT_SM(kSrc)                                                          \
    /* Compile state machine, specifying not to rake the assembly. */          \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(                               \
        smSrc,                                                                 \
        svAsm,                                                                 \
        smAsm,                                                                 \
        nullptr,                                                               \
        StateMachineCompiler::FIRST_STATE,                                     \
        false));                                                               \
                                                                               \
    /* Get state machine. */                                                   \
    StateMachine& sm = smAsm->get();                                           \
                                                                               \
    /* Get local state vector. */                                              \
    StateVector& localSv = smAsm->localStateVector();

///
/// @brief Initializes the state script.
///
/// @param[in] kSrc  State script config as string.
///
#define INIT_SS(kSrc)                                                          \
    std::stringstream ssSrc(kSrc);                                             \
    Ref<StateScriptAssembly> ssAsm;                                            \
    ErrorInfo ssTokInfo{};                                                     \
    CHECK_SUCCESS(StateScriptCompiler::compile(ssSrc,                          \
                                               smAsm,                          \
                                               ssAsm,                          \
                                               &ssTokInfo));

///
/// @brief Checks the value of a state vector element.
///
/// @param[in] kElemName   Element name.
/// @param[in] kElemType   Element type identifier.
/// @param[in] kExpectVal  Expected value
///
#define CHECK_SV_ELEM(kElemName, kElemType, kExpectVal)                        \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv.getElement(kElemName, _elem));                            \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

///
/// @brief Checks the value of a state machine local element.
///
/// @param[in] kElemName   Element name.
/// @param[in] kElemType   Element type identifier.
/// @param[in] kExpectVal  Expected value
///
#define CHECK_LOCAL_ELEM(kElemName, kElemType, kExpectVal)                     \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(localSv.getElement(kElemName, _elem));                       \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

///
/// @brief Checks that compiling a state script generates a certain error.
///
/// @param[in] kSrc      State script config.
/// @param[in] kSmAsm    State machine.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error line number.
/// @param[in] kColNum   Expected error column number.
///
static void checkCompileError(std::stringstream& kSrc,
                              const Ref<const StateMachineAssembly> kSmAsm,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Make a copy of the original source for the 2nd compiler call.
    std::stringstream ssCpy(kSrc.str());

    // Got expected return code from compiler.
    Ref<StateScriptAssembly> ssAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateScriptCompiler::compile(kSrc,
                                                   kSmAsm,
                                                   ssAsm,
                                                   &err));

    // Assembly pointer was not populated.
    CHECK_TRUE(ssAsm == nullptr);

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateScriptCompiler::compile(ssCpy,
                                                   kSmAsm,
                                                   ssAsm,
                                                   nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for StateScriptCompiler.
///
TEST_GROUP(StateScriptCompiler)
{
};

///
/// @test State script that runs for a single step and passes.
///
TEST(StateScriptCompiler, SingleStepPass)
{
    // General logic: state script executes for a single step. The state machine
    // increments element `bar` when element `foo` is true; `foo` is set via
    // a state script input.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo: bar = bar + 1\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = true\n"
        "    @assert bar == 1\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 0);
}

///
/// @test State script that runs for a single step and fails.
///
TEST(StateScriptCompiler, SingleStepFail)
{
    // General logic: same as in `SingleStepPass`, except state script expects
    // a different value for `bar` that is not met.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo: bar = bar + 1\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = true\n"
        "    @assert bar == 2\n" // Assert fails on T=0
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(false, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(0, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Location of failed assert was correctly identified.
    CHECK_EQUAL(7, ssTokInfo.lineNum);
    CHECK_EQUAL(5, ssTokInfo.colNum);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 0);
}

///
/// @test State script that runs for multiple steps and passes.
///
TEST(StateScriptCompiler, MultiStepPass)
{
    // General logic: element `bar` is updated according to some basic logic
    // that references a state vector element and the current time. State script
    // checks the value of `bar` at each step from T=0 to T=10.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    T < 5 {\n"
        "        foo: bar = bar + 1\n"
        "        else: bar = -1\n"
        "    }\n"
        "    else: bar = bar + 2\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = false\n"
        "    @assert bar == -1\n"
        "}\n"
        "T == 1 {\n"
        "    foo = true\n"
        "    @assert bar == 0\n"
        "}\n"
        "T == 2: @assert bar == 1\n"
        "T == 3 {\n"
        "    foo = false\n"
        "    @assert bar == -1\n"
        "}\n"
        "T == 4 {\n"
        "    foo = true\n"
        "    @assert bar == 0\n"
        "}\n"
        "T >= 5 {\n"
        "    foo = false\n"
        "    @assert bar == 2 * (T - 4)\n"
        "}\n"
        "T == 10: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(11, report.steps);
    CHECK_EQUAL(11, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 10);
    CHECK_SV_ELEM("foo", bool, false);
    CHECK_LOCAL_ELEM("bar", I32, 12);
    CHECK_LOCAL_ELEM("T", U64, 10);
}

///
/// @test State script that runs for multiple steps and fails.
///
TEST(StateScriptCompiler, MultiStepFail)
{
    // General logic: same as in `MultiStepPass`, except state machine fails to
    // update element `bar` on T=8.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    T < 5 {\n"
        "        foo: bar = bar + 1\n"
        "        else: bar = -1\n"
        "    }\n"
        "    else: T != 8: bar = bar + 2\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = false\n"
        "    @assert bar == -1\n"
        "}\n"
        "T == 1 {\n"
        "    foo = true\n"
        "    @assert bar == 0\n"
        "}\n"
        "T == 2: @assert bar == 1\n"
        "T == 3 {\n"
        "    foo = false\n"
        "    @assert bar == -1\n"
        "}\n"
        "T == 4 {\n"
        "    foo = true\n"
        "    @assert bar == 0\n"
        "}\n"
        "T >= 5 {\n"
        "    foo = false\n"
        "    @assert bar == 2 * (T - 4)\n" // Assert fails on T=8
        "}\n"
        "T == 10: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(false, report.pass);
    CHECK_EQUAL(9, report.steps);
    CHECK_EQUAL(8, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Location of failed assert was correctly identified.
    CHECK_EQUAL(24, ssTokInfo.lineNum);
    CHECK_EQUAL(5, ssTokInfo.colNum);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 8);
    CHECK_SV_ELEM("foo", bool, false);
    CHECK_LOCAL_ELEM("bar", I32, 6);
    CHECK_LOCAL_ELEM("T", U64, 8);
}

///
/// @test State script with a delta T that is not 1.
///
TEST(StateScriptCompiler, DeltaT)
{
    // General logic: state script steps from T=0 to T=9 with a delta T of 3.
    // State machine sums the values of T in an accumulator. Expect a sum of
    // 18 = 0 + 3 + 6 + 9.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[local]\n"
        "U64 sum = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    sum = sum + T\n");
    INIT_SS(
        "[options]\n"
        "delta_t 3\n"
        "\n"
        "[Initial]\n"
        "T == 9: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(4, report.steps);
    CHECK_EQUAL(0, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 9);
    CHECK_LOCAL_ELEM("sum", U64, 18);
    CHECK_LOCAL_ELEM("T", U64, 9);
}

///
/// @test State time element is updated correctly in a state script.
///
TEST(StateScriptCompiler, StateTime)
{
    // General logic: state `Initial` loops every 6 steps. In the state, element
    // `foo` is false on even steps and true on odd steps. State script checks
    // the value of `foo` at each step and stops just before the state loops a
    // 3rd time.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[local]\n"
        "bool foo = false\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo = (T == 1 or T == 3 or T == 5)\n"
        "    T == 5: -> Initial\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0: @assert !foo\n"
        "T == 1: @assert foo\n"
        "T == 2: @assert !foo\n"
        "T == 3: @assert foo\n"
        "T == 4: @assert !foo\n"
        "T == 5: @assert foo\n"
        "G == 11: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(12, report.steps);
    CHECK_EQUAL(12, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 11);
    CHECK_LOCAL_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("T", U64, 5);
}

///
/// @test Variant of the `StateTime` test with a state script that fails.
///
TEST(StateScriptCompiler, StateTimeFail)
{
    // General logic: same as in `StateTime`, but the state machine sets an
    // incorrect value for element `foo` on T=4 on the 2nd loop of the state.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[local]\n"
        "bool foo = false\n"
        "bool looped = false\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo = (T == 1 or T == 3 or T == 5)\n"
        "    looped and T == 4: foo = true\n"
        "    T == 5: -> Initial\n"
        ".exit\n"
        "    looped = true\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 0: @assert !foo\n"
        "T == 1: @assert foo\n"
        "T == 2: @assert !foo\n"
        "T == 3: @assert foo\n"
        "T == 4: @assert !foo\n"
        "T == 5: @assert foo\n"
        "G == 11: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(false, report.pass);
    CHECK_EQUAL(11, report.steps);
    CHECK_EQUAL(10, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Location of failed assert was correctly identified.
    CHECK_EQUAL(9, ssTokInfo.lineNum);
    CHECK_EQUAL(9, ssTokInfo.colNum);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 10);
    CHECK_LOCAL_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("looped", bool, true);
    CHECK_LOCAL_ELEM("T", U64, 4);
}

///
/// @test State script with a state machine that has multiple states.
///
TEST(StateScriptCompiler, MultiState)
{
    // General logic: states `Foo` and `Bar` transition to one another when
    // element `trans` is true; this element is set by the state script. Each
    // state has a unique behavior and a shared behavior. The state script
    // checks for the unique behavior in state-specific sections and the shared
    // behavior in the all states section.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool trans\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool trans\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 1\n"
        ".step\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".exit\n"
        "    trans = false\n"
        "\n"
        "[Bar]\n"
        ".entry\n"
        "    bar = foo\n"
        ".step\n"
        "    baz = T / 2\n"
        "    trans: -> Foo\n"
        "    bar = bar + 1\n"
        ".exit\n"
        "    trans = false\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[all_states]\n"
        "true: @assert baz == T / 2\n"
        "\n"
        "[Foo]\n"
        "T == 0: @assert foo == 2\n"
        "T == 1: @assert foo == 4\n"
        "T == 2: @assert foo == 8\n"
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @assert bar == 9\n"
        "T == 1: @assert bar == 10\n"
        "T == 2: @assert bar == 11\n"
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert bar == 11\n"
        "    @assert foo == 8\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(8, report.steps);
    CHECK_EQUAL(17, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_SV_ELEM("time", U64, 7);
    CHECK_SV_ELEM("trans", bool, false);
    CHECK_LOCAL_ELEM("foo", I32, 8);
    CHECK_LOCAL_ELEM("bar", I32, 11);
    CHECK_LOCAL_ELEM("baz", F64, (3.0 / 2.0));
}

///
/// @test Multi-state state script that fails an assertion in a state section.
///
TEST(StateScriptCompiler, MultiStateFailInStateSection)
{
    // General logic: same as in `MultiState`, but state `Bar` fails to execute
    // its unique behavior on T=2.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool trans\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool trans\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 1\n"
        ".step\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".exit\n"
        "    trans = false\n"
        "\n"
        "[Bar]\n"
        ".entry\n"
        "    bar = foo\n"
        ".step\n"
        "    baz = T / 2\n"
        "    trans: -> Foo\n"
        "    T != 2: bar = bar + 1\n"
        ".exit\n"
        "    trans = false\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[all_states]\n"
        "true: @assert baz == T / 2\n"
        "\n"
        "[Foo]\n"
        "T == 0: @assert foo == 2\n"
        "T == 1: @assert foo == 4\n"
        "T == 2: @assert foo == 8\n"
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @assert bar == 9\n"
        "T == 1: @assert bar == 10\n"
        "T == 2: @assert bar == 11\n" // Failing assert
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert bar == 11\n"
        "    @assert foo == 8\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(false, report.pass);
    CHECK_EQUAL(7, report.steps);
    CHECK_EQUAL(13, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Location of failed assert was correctly identified.
    CHECK_EQUAL(19, ssTokInfo.lineNum);
    CHECK_EQUAL(9, ssTokInfo.colNum);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_SV_ELEM("time", U64, 6);
    CHECK_SV_ELEM("trans", bool, false);
    CHECK_LOCAL_ELEM("foo", I32, 8);
    CHECK_LOCAL_ELEM("bar", I32, 10);
    CHECK_LOCAL_ELEM("baz", F64, 1.0);
}

///
/// @test Multi-state state script that fails an assertion in the all states
/// section.
///
TEST(StateScriptCompiler, MultiStateFailInAllStatesSection)
{
    // General logic: same as in `MultiState`, but state `Bar` fails to execute
    // its shared behavior on T=2.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool trans\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool trans\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 1\n"
        ".step\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".exit\n"
        "    trans = false\n"
        "\n"
        "[Bar]\n"
        ".entry\n"
        "    bar = foo\n"
        ".step\n"
        "    T != 2: baz = T / 2\n"
        "    trans: -> Foo\n"
        "    bar = bar + 1\n"
        ".exit\n"
        "    trans = false\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[all_states]\n"
        "true: @assert baz == T / 2\n" // Failing assert
        "\n"
        "[Foo]\n"
        "T == 0: @assert foo == 2\n"
        "T == 1: @assert foo == 4\n"
        "T == 2: @assert foo == 8\n"
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @assert bar == 9\n"
        "T == 1: @assert bar == 10\n"
        "T == 2: @assert bar == 11\n"
        "T == 3 {\n"
        "    trans = true\n"
        "    @assert bar == 11\n"
        "    @assert foo == 8\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(false, report.pass);
    CHECK_EQUAL(7, report.steps);
    CHECK_EQUAL(12, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Location of failed assert was correctly identified.
    CHECK_EQUAL(5, ssTokInfo.lineNum);
    CHECK_EQUAL(7, ssTokInfo.colNum);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_SV_ELEM("time", U64, 6);
    CHECK_SV_ELEM("trans", bool, false);
    CHECK_LOCAL_ELEM("foo", I32, 8);
    CHECK_LOCAL_ELEM("bar", I32, 11);
    CHECK_LOCAL_ELEM("baz", F64, (1.0 / 2.0));
}

///
/// @test State scripts may use element aliases in assertions.
///
TEST(StateScriptCompiler, UseAliasInAssert)
{
    // General logic: state `Initial` increments element `foo` indefinitely.
    // `foo` is aliased to `bar`.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo @alias bar\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo = foo + 1\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "T == 10 {\n"
        "    @assert bar == 11\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(11, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 10);
    CHECK_SV_ELEM("foo", I32, 11);
}

///
/// @test State scripts may use element aliases in assignment statements.
///
TEST(StateScriptCompiler, UseAliasInInput)
{
    // General logic: state `Initial` sets element `bar` to true when `foo` is
    // true. `foo` is aliased to `baz`.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n"
        "bool bar\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo @alias baz\n"
        "bool bar\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo: bar = true\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "true {\n"
        "    baz = true\n"
        "    @assert bar == true\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, true);
    CHECK_SV_ELEM("bar", bool, true);
}

///
/// @test State scripts may use element aliases in conditionals.
///
TEST(StateScriptCompiler, UseAliasInGuard)
{
    // General logic: state `Initial` sets element `foo` to true on T=5. `foo`
    // is aliased to `bar`. State script stops when `bar` is true.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo @alias bar\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    T == 5: foo = true\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "bar {\n"
        "    @assert T == 6\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(7, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 6);
    CHECK_SV_ELEM("foo", bool, true);
}

///
/// @test State scripts that use stats functions update the expression stats.
///
TEST(StateScriptCompiler, UpdateExpressionStats)
{
    // General logic: state `Initial` sets element `foo` to various values for
    // the first 3 steps. The state script stops when the rolling max of `foo`
    // hits a certain value.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    T == 0: foo = 3\n"
        "    T == 1: foo = 2\n"
        "    T == 2: foo = 1\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Initial]\n"
        "roll_max(foo, 2) == 2 {\n"
        "    @assert T == 3\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(4, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 3);
    CHECK_SV_ELEM("foo", I32, 1);
}

///
/// @test State script that specifies an initial state other than the first
/// defined.
///
TEST(StateScriptCompiler, ConfigInitialState)
{
    // General logic: states `Foo` and `Bar` are terminal states. `Foo` sets
    // element `foo` to true. The state script specifies `Bar` as the initial
    // state and stops immediately. Expect state machine to end in `Bar` with
    // `foo` remaining false.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "bool foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "bool foo\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = true\n"
        "\n"
        "[Bar]\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "init_state Bar\n"
        "\n"
        "[all_states]\n"
        "true: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(0, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, false);
}

///
/// @test State script with an empty state section.
///
TEST(StateScriptCompiler, EmptyStateSection)
{
    // General logic: states `Foo` and `Bar` are terminal states. `Foo` sets
    // element `foo` to true. The state script specifies `Bar` as the initial
    // state and stops immediately. Expect state machine to end in `Bar` with
    // `foo` remaining false.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n"
        "\n"
        "[Bar]\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "\n"
        "[all_states]\n"
        "true: @stop\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(0, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
}

///
/// @test State script statements affect the program state seen by successive
/// statements within the same step.
///
TEST(StateScriptCompiler, ImperativeInputs)
{
    // General logic: state machine has no interesting logic, state script has
    // a chain of guarded inputs that trigger each other and stop the script
    // after one step.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Foo]\n");
    INIT_SS(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[all_states]\n"
        "true: foo = 1\n"
        "foo == 1: foo = 2\n"
        "foo == 2: foo = 3\n"
        "foo == 3 {\n"
        "    @assert true\n"
        "    @stop\n"
        "}\n");

    // Run state script.
    StateScriptAssembly::Report report{};
    CHECK_SUCCESS(ssAsm->run(ssTokInfo, report));

    // Report contains expected data.
    CHECK_EQUAL(true, report.pass);
    CHECK_EQUAL(1, report.steps);
    CHECK_EQUAL(1, report.asserts);
    CHECK_TRUE(report.text.size() > 0);

    // Final state vector contains expected values.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
}

///////////////////////////////// Error Tests //////////////////////////////////

///
/// @brief Unit tests for StateScriptCompiler errors.
///
TEST_GROUP(StateScriptCompilerErrors)
{
};

///
/// @test Passing a null parse to the compiler generates an error.
///
TEST(StateScriptCompilerErrors, NullParse)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    Ref<StateScriptAssembly> ssAsm;
    Ref<const StateScriptParse> ssParse;
    CHECK_ERROR(E_SSC_NULL, StateScriptCompiler::compile(ssParse,
                                                         smAsm,
                                                         ssAsm,
                                                         nullptr));
    CHECK_TRUE(ssAsm == nullptr);
}

///
/// @test Two state sections of the same name generates an error.
///
TEST(StateScriptCompilerErrors, DupeSection)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "[Foo]\n");
    checkCompileError(ss, smAsm, E_SSC_DUPE, 5, 1);
}

///
/// @test State section with an unknown state generates an error.
///
TEST(StateScriptCompilerErrors, UnknownState)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Bar]\n");
    checkCompileError(ss, smAsm, E_SSC_STATE, 4, 1);
}

///
/// @test Assignment statement with no condition generates an error.
///
TEST(StateScriptCompilerErrors, UnguardedInput)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

///
/// @test Assertion with no condition generates an error.
///
TEST(StateScriptCompilerErrors, UnguardedAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@assert foo == 0\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

///
/// @test Stop annotation with no condition generates an error.
///
TEST(StateScriptCompilerErrors, UnguardedStop)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@stop\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

///
/// @test Illegal else keyword generates an error.
///
TEST(StateScriptCompilerErrors, IllegalElse)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "T == 0: foo = 1\n"
        "else: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSC_ELSE, 6, 1);
}

///
/// @test Errors in conditional expressions are surfaced.
///
TEST(StateScriptCompilerErrors, SurfaceErrorInGuardExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "bar == 1: foo = 1\n");
    checkCompileError(ss, smAsm, E_EXC_ELEM, 5, 1);
}

///
/// @test Nested conditionals generate an error.
///
TEST(StateScriptCompilerErrors, NestedGuard)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo == 1: T == 0: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSC_NEST, 5, 11);
}

///
/// @test An unreachable assignment statement after a stop annotation generates
/// an error.
///
TEST(StateScriptCompilerErrors, UnreachableInput)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true {\n"
        "    @stop\n"
        "    foo = 1\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSC_UNRCH, 7, 5);
}

///
/// @test An unreachable assertion after a stop annotation generates an error.
///
TEST(StateScriptCompilerErrors, UnreachableAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true {\n"
        "    @stop\n"
        "    @assert foo == 0\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSC_UNRCH, 7, 5);
}

///
/// @test Errors in assertion expressions are surfaced.
///
TEST(StateScriptCompilerErrors, SurfaceErrorInAssertExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: @assert bar == 1\n");
    checkCompileError(ss, smAsm, E_EXC_ELEM, 5, 15);
}

///
/// @test Errors in asignment statements are surfaced.
///
TEST(StateScriptCompilerErrors, SurfaceErrorInAction)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: bar = 1\n");
    checkCompileError(ss, smAsm, E_SMC_ASG_ELEM, 5, 7);
}

///
/// @test A state script with no stop annotation generates an error.
///
TEST(StateScriptCompilerErrors, NoStop)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: foo = 1\n");
    checkCompileError(ss, smAsm, E_SSC_STOP, -1, -1);
}

///
/// @test Running a state script with a very large delta T eventually generates
/// an error when the global time element overflows.
///
TEST(StateScriptCompilerErrors, GlobalClockOverflow)
{
    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Initial]\n");
    INIT_SS(
        "[options]\n"
        "delta_t 9223372036854775806\n" // I64 max value - 1
        "\n"
        "[Initial]\n"
        "T == 3: @stop\n");

    // Run state script. Expect an error due to global clock overflow.
    StateScriptAssembly::Report report{};
    CHECK_ERROR(E_SSC_OVFL, ssAsm->run(ssTokInfo, report));
}

///
/// @test Specifying a floating delta T generates an error.
///
TEST(StateScriptCompilerErrors, DeltaTFloating)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1.5\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

///
/// @test Specifying a negative delta T generates an error.
///
TEST(StateScriptCompilerErrors, DeltaTNegative)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t -1\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

///
/// @test Specifying a too large of a delta T generates an error.
///
TEST(StateScriptCompilerErrors, DeltaTTooLarge)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 999999999999999999999999999999999999999999999999999999999999\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

///
/// @test Specifying an unknown initial state generates an error.
///
TEST(StateScriptCompilerErrors, UnknownInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[options]\n"
        "delta_t 1\n"
        "init_state Bar\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_STATE, 3, 12);
}

///
/// @test Providing a raked state machine assembly to the state script compiler
/// generates an error.
///
TEST(StateScriptCompilerErrors, RakedStateMachineAssembly)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");

    // Compile state machine, specifying to rake the assembly.
    std::stringstream smSrc(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "U64 time @alias G\n"
        "\n"
        "[Foo]\n");
    Ref<const StateMachineAssembly> smAsm;
    CHECK_SUCCESS(StateMachineCompiler::compile(
        smSrc, svAsm, smAsm, nullptr, StateMachineCompiler::FIRST_STATE, true));

    std::stringstream ssSrc(
        "[options]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    Ref<StateScriptAssembly> ssAsm;
    CHECK_ERROR(E_SSC_RAKE,
                StateScriptCompiler::compile(ssSrc,smAsm, ssAsm, nullptr));
}
