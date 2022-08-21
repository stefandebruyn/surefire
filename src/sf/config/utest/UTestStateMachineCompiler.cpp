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
/// @file  sf/config/utest/UTestStateMachineCompiler.hpp
/// @brief Unit tests for StateMachineCompiler.
////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

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
    StateVector& sv = svAsm->get();                                            \
    (void) sv;

///
/// @brief Initializes the state machine.
///
/// @param[in] kSrc  State machine config as string.
///
#define INIT_SM(kSrc)                                                          \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(smSrc, svAsm, smAsm, nullptr));\
                                                                               \
    /* Get state machine. */                                                   \
    StateMachine& sm = smAsm->get();                                           \
    (void) sm;

///
/// @brief Sets a state vector element.
///
/// @param[in] kElemName  Element name.
/// @param[in] kElemType  Element type identifier.
/// @param[in] kSetVal    New value.
///
#define SET_SV_ELEM(kElemName, kElemType, kSetVal)                             \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv.getElement(kElemName, _elem));                            \
    _elem->write(kSetVal);                                                     \
}

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
    StateVector& _localSv = smAsm->localStateVector();                         \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(_localSv.getElement(kElemName, _elem));                      \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

///
/// @brief Checks that compiling a state machine generates a certain error.
///
/// @param[in] kParse    State machine parse.
/// @param[in] kSvAsm    State vector.
/// @param[in] kRes      Expected error code.
/// @param[in] kLineNum  Expected error line number.
/// @param[in] kColNum   Expected error column number.
///
static void checkCompileError(const Ref<const StateMachineParse> kParse,
                              const Ref<const StateVectorAssembly> kSvAsm,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Got expected return code from compiler.
    Ref<const StateMachineAssembly> smAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineCompiler::compile(kParse,
                                                    kSvAsm,
                                                    smAsm,
                                                    &err));

    // Assembly pointer was not populated.
    CHECK_TRUE(smAsm == nullptr);

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // A null error info pointer is not dereferenced.
    CHECK_ERROR(kRes, StateMachineCompiler::compile(kParse,
                                                    kSvAsm,
                                                    smAsm,
                                                    nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

///
/// @brief Unit tests for StateMachineCompiler.
///
TEST_GROUP(StateMachineCompiler)
{
};

///
/// @test Entry label compiles correctly.
///
TEST(StateMachineCompiler, EntryLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G \n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 1\n");

    // Element `foo` is set to 1 in the initial state entry label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_LOCAL_ELEM("foo", I32, 1);
}

///
/// @test Step label compiles correctly.
///
TEST(StateMachineCompiler, StepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo = foo + 1\n");

    // Element `foo` is incremented twice in the initial state step label.
    CHECK_SUCCESS(sm.step());
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

///
/// @test Transitioning in an entry label executes the exit label.
///
TEST(StateMachineCompiler, TransitionInEntryDoExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    -> Foo\n"
        ".exit\n"
        "    foo = 1\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 2\n");

    // Element `foo` is set to 1 in the initial state exit label.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

///
/// @test Transitioning in a step label executes the exit label.
///
TEST(StateMachineCompiler, TransitionInStepDoExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    -> Foo\n"
        ".exit\n"
        "    foo = 1\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 2\n");

    // Element `foo` is set to 1 in the initial state exit label.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

///
/// @test If branch of a conditional is taken when the condition is true.
///
TEST(StateMachineCompiler, GuardTakeIfBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    true: foo = 1\n"
        "    else: foo = 2\n"
        "    bar = 1\n");

    // Element `foo` is set to 1 in the if branch.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // Element `bar` is set to 1 regardless of the branch taken.
    CHECK_LOCAL_ELEM("bar", I32, 1);
}

///
/// @test Else branch of a conditional is taken when the condition is false.
///
TEST(StateMachineCompiler, GuardTakeElseBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    false: foo = 1\n"
        "    else: foo = 2\n"
        "    bar = 1\n");

    // Element `foo` is set to 2 in the else branch.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);

    // Element `bar` is set to 1 regardless of the branch taken.
    CHECK_LOCAL_ELEM("bar", I32, 1);
}

///
/// @test Aliases can be used in place of the aliased element name.
///
TEST(StateMachineCompiler, UseAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n"
        "I32 baz\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo @alias bar\n"
        "I32 baz @alias qux\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    baz = 1\n"
        "    bar = qux\n");

    // Element `foo` is set to 1 in the initial state entry label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 1);
}

///
/// @test A state machine with all element types compiles correctly.
///
TEST(StateMachineCompiler, AllElementTypes)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I8 a\n"
        "I16 b\n"
        "I32 c\n"
        "I64 d\n"
        "U8 e\n"
        "U16 f\n"
        "U32 g\n"
        "U64 h\n"
        "F32 i\n"
        "F64 j\n"
        "bool k\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I8 a\n"
        "I16 b\n"
        "I32 c\n"
        "I64 d\n"
        "U8 e\n"
        "U16 f\n"
        "U32 g\n"
        "U64 h\n"
        "F32 i\n"
        "F64 j\n"
        "bool k\n"
        "I32 foo\n"
        "\n"
        "[local]\n"
        "I8 l = 0\n"
        "I16 m = 0\n"
        "I32 n = 0\n"
        "I64 o = 0\n"
        "U8 p = 0\n"
        "U16 q = 0\n"
        "U32 r = 0\n"
        "U64 s = 0\n"
        "F32 t = 0\n"
        "F64 u = 0\n"
        "bool v = 0\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    a = 1\n"
        "    b = 1\n"
        "    c = 1\n"
        "    d = 1\n"
        "    e = 1\n"
        "    f = 1\n"
        "    g = 1\n"
        "    h = 1\n"
        "    i = 1\n"
        "    j = 1\n"
        "    k = true\n"
        "    l = 1\n"
        "    m = 1\n"
        "    n = 1\n"
        "    o = 1\n"
        "    p = 1\n"
        "    q = 1\n"
        "    r = 1\n"
        "    s = 1\n"
        "    t = 1\n"
        "    u = 1\n"
        "    v = true\n"
        ".step\n"
        "    foo = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o"
        "          + p + q + r + s + t + u + v\n");

    // Element `foo` is set to 22 in the initial state step label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 22);
}

///
/// @test The global time, state time, and state elements are updated correctly.
///
TEST(StateMachineCompiler, SpecialElements)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    T == 10: -> Foo\n"
        "\n"
        "[Foo]\n");

    // State and global time is initially 0.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_LOCAL_ELEM("T", U64, 0);
    CHECK_SV_ELEM("state", U32, 1);

    // Step at time 9. Transition to state `Foo` does not occur yet.
    SET_SV_ELEM("time", U64, 9);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 9);
    CHECK_LOCAL_ELEM("T", U64, 9);
    CHECK_SV_ELEM("state", U32, 1);

    // Step at time 10. State machine remains in initial state and will
    // transition on next step.
    SET_SV_ELEM("time", U64, 10);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 10);
    CHECK_LOCAL_ELEM("T", U64, 10);
    CHECK_SV_ELEM("state", U32, 1);

    // Step at time 11. State element updates, state time element resets.
    SET_SV_ELEM("time", U64, 11);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 11);
    CHECK_LOCAL_ELEM("T", U64, 0);
    CHECK_SV_ELEM("state", U32, 2);
}

///
/// @test Stats function is computed correctly.
///
TEST(StateMachineCompiler, StatsFunctionUsingStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    bar = roll_avg(foo, 2)\n");

    SET_SV_ELEM("foo", I32, 3);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("bar", I32, 3);

    SET_SV_ELEM("foo", I32, 5);
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("bar", I32, 4);

    SET_SV_ELEM("foo", I32, 7);
    SET_SV_ELEM("time", U64, 2);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("bar", I32, 6);
}

///
/// @test Transitioning to the current state restarts it.
///
TEST(StateMachineCompiler, TransitionToCurrentState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n"
        "I32 bar\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "I32 bar\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 0\n"
        ".step\n"
        "    foo = foo + 1\n"
        "    foo == 3: -> Initial\n"
        ".exit\n"
        "    bar = bar + 1\n");

    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 1);
    CHECK_SV_ELEM("bar", I32, 0);
    CHECK_LOCAL_ELEM("T", U64, 0);

    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 2);
    CHECK_SV_ELEM("bar", I32, 0);
    CHECK_LOCAL_ELEM("T", U64, 1);

    SET_SV_ELEM("time", U64, 2);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 3);
    CHECK_SV_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 2);

    SET_SV_ELEM("time", U64, 3);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 1);
    CHECK_SV_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 0);
}

///
/// @test Local elements of all types have the correct initial values.
///
TEST(StateMachineCompiler, LocalElementInitialValues)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I8 a = 1\n"
        "I16 b = 2\n"
        "I32 c = 3\n"
        "I64 d = 4\n"
        "U8 e = 5\n"
        "U16 f = 6\n"
        "U32 g = 7\n"
        "U64 h = 8\n"
        "F32 i = 9\n"
        "F64 j = 10\n"
        "bool k = true\n"
        "\n"
        "[Initial]\n");

    CHECK_LOCAL_ELEM("a", I8, 1);
    CHECK_LOCAL_ELEM("b", I16, 2);
    CHECK_LOCAL_ELEM("c", I32, 3);
    CHECK_LOCAL_ELEM("d", I64, 4);
    CHECK_LOCAL_ELEM("e", U8, 5);
    CHECK_LOCAL_ELEM("f", U16, 6);
    CHECK_LOCAL_ELEM("g", U32, 7);
    CHECK_LOCAL_ELEM("h", U64, 8);
    CHECK_LOCAL_ELEM("i", F32, 9.0f);
    CHECK_LOCAL_ELEM("j", F64, 10.0);
    CHECK_LOCAL_ELEM("k", bool, true);
}

///
/// @test Local element initial value expressions may reference previously
/// assigned local elements.
///
TEST(StateMachineCompiler, InitLocalElemsWithLocalElems)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 1\n"
        "I32 bar = foo + 1\n"
        "I32 baz = bar + 1\n"
        "\n"
        "[Initial]\n");

    CHECK_LOCAL_ELEM("foo", I32, 1);
    CHECK_LOCAL_ELEM("bar", I32, 2);
    CHECK_LOCAL_ELEM("baz", I32, 3);
}

///
/// @test Element assignment performs a safe cast.
///
TEST(StateMachineCompiler, AssignmentDoesSafeCast)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I8 a\n"
        "I16 b\n"
        "I32 c\n"
        "I64 d\n"
        "U8 e\n"
        "U16 f\n"
        "U32 g\n"
        "U64 h\n"
        "F32 i\n"
        "F64 j\n"
        "bool k\n");
    INIT_SM(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I8 a\n"
        "I16 b\n"
        "I32 c\n"
        "I64 d\n"
        "U8 e\n"
        "U16 f\n"
        "U32 g\n"
        "U64 h\n"
        "F32 i\n"
        "F64 j\n"
        "bool k\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    a = 1\n"
        "    b = 1\n"
        "    c = 1\n"
        "    d = 1\n"
        "    e = 1\n"
        "    f = 1\n"
        "    g = 1\n"
        "    h = 1\n"
        "    i = 1\n"
        "    j = 1\n"
        "    k = true\n"
        ".step\n"
        "    a = 0 / 0\n"
        "    b = 0 / 0\n"
        "    c = 0 / 0\n"
        "    d = 0 / 0\n"
        "    e = 0 / 0\n"
        "    f = 0 / 0\n"
        "    g = 0 / 0\n"
        "    h = 0 / 0\n"
        "    i = 0 / 0\n"
        "    j = 0 / 0\n"
        "    k = 0 / 0\n");

    CHECK_SUCCESS(sm.step());

    CHECK_SV_ELEM("a", I8, 0);
    CHECK_SV_ELEM("b", I16, 0);
    CHECK_SV_ELEM("c", I32, 0);
    CHECK_SV_ELEM("d", I64, 0);
    CHECK_SV_ELEM("e", U8, 0);
    CHECK_SV_ELEM("f", U16, 0);
    CHECK_SV_ELEM("g", U32, 0);
    CHECK_SV_ELEM("h", U64, 0);
    CHECK_SV_ELEM("i", F32, 0.0f);
    CHECK_SV_ELEM("j", F64, 0.0);
    CHECK_SV_ELEM("k", bool, false);
}

///
/// @test An initial state other than the first defined state may be specified.
///
TEST(StateMachineCompiler, SpecifyInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    std::stringstream smSrc(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    foo = 10\n"
        "\n"
        "[Bar]\n"
        ".entry\n"
        "    foo = 100\n");
    Ref<const StateMachineAssembly> smAsm;
    CHECK_SUCCESS(
        StateMachineCompiler::compile(smSrc, svAsm, smAsm, nullptr, "Bar"));
    StateMachine& sm = smAsm->get();
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 100);
}

///////////////////////////////// Error Tests //////////////////////////////////

///
/// @brief Unit tests for StateMachineCompiler errors.
///
TEST_GROUP(StateMachineCompilerErrors)
{
};

///
/// @test Referencing an unknown element in the state vector section generates
/// an error.
///
TEST(StateMachineCompilerErrors, UnknownStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "I32 bar\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_SV_ELEM, 2, 5);
}

///
/// @test Referencing an unknown type in the state vector section generates
/// an error.
///
TEST(StateMachineCompilerErrors, UnknownStateVectorElementType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "I33 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_TYPE, 2, 1);
}

///
/// @test Referencing an element by the wrong type in the state vector section
/// generates an error.
///
TEST(StateMachineCompilerErrors, StateVectorElementTypeMismatch)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "F32 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_TYPE_MISM, 2, 1);
}

///
/// @test Referencing an element twice in the state vector section generates an
/// error.
///
TEST(StateMachineCompilerErrors, StateVectorElementListedTwice)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_DUPE, 3, 5);
}

///
/// @test Using a non-U64 element as the global time generates an error.
///
TEST(StateMachineCompilerErrors, GlobalTimeElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias G\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_G_TYPE, 2, 5);
}

///
/// @test Using a non-U32 element as the state element generates an error.
///
TEST(StateMachineCompilerErrors, StateElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "I32 foo @alias S\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_S_TYPE, 2, 5);
}

///
/// @test Reusing a state vector element name for a local element generates an
/// error.
///
TEST(StateMachineCompilerErrors, LocalElementReusesSvElementName)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[local]\n"
        "F64 foo = 0\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_DUPE, 7, 5);
}

///
/// @test Assigning an unknown element generates an error.
///
TEST(StateMachineCompilerErrors, AssignmentActionUnknownElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 7, 5);
}

///
/// @test Referencing an unknown element in an assignment expression generates
/// an error.
///
TEST(StateMachineCompilerErrors, ErrorInAssignmentActionExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = bar\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_EXC_ELEM, 8, 11);
}

///
/// @test Transitioning to an unknown state generates an error.
///
TEST(StateMachineCompilerErrors, TransitionToUnknownState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    -> Foo\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_STATE, 7, 8);
}

///
/// @test Errors in conditional expressions are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInGuardExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    bar: foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_EXC_ELEM, 8, 5);
}

///
/// @test Errors in if branches are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInIfBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo == 0: bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 8, 15);
}

///
/// @test Errors in else branches are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInElseBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo == 0: foo = 1\n"
        "    else: bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 9, 11);
}

///
/// @test Errors further down the block tree are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInNextBlock)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo == 0: foo = 1\n"
        "    else: foo = 2\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 10, 5);
}

///
/// @test Errors in the step label are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInStepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".step\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 7, 5);
}

///
/// @test Errors in the exit label are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".exit\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 7, 5);
}

///
/// @test Errors in a state that is not the first defined are surfaced.
///
TEST(StateMachineCompilerErrors, ErrorInNonInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 1\n"
        "    -> Foo\n"
        "\n"
        "[Foo]\n"
        ".entry\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASG_ELEM, 13, 5);
}

///
/// @test Not aliasing a global time element generates an error.
///
TEST(StateMachineCompilerErrors, NoGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_NO_G, -1, -1);
}

///
/// @test Not aliasing a state element generates an error.
///
TEST(StateMachineCompilerErrors, NoStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_NO_S, -1, -1);
}

///
/// @test Writing a read-only state vector element generates an error.
///
TEST(StateMachineCompilerErrors, WriteReadOnlyStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo @read_only\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 8, 5);
}

///
/// @test Writing a read-only state vector element by its alias generates an
/// error.
///
TEST(StateMachineCompilerErrors, WriteReadOnlyStateVectorElementAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo @read_only @alias bar\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 8, 5);
}

///
/// @test Writing a read-only local element generates an error.
///
TEST(StateMachineCompilerErrors, WriteReadOnlyLocalElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = 0 @read_only\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 10, 5);
}

///
/// @test Writing the global time element generates an error.
///
TEST(StateMachineCompilerErrors, WriteGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    G = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 7, 5);
}

///
/// @test Writing the state time element generates an error.
///
TEST(StateMachineCompilerErrors, WriteLocalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    T = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 7, 5);
}

///
/// @test Writing the state element generates an error.
///
TEST(StateMachineCompilerErrors, WriteStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    S = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ELEM_RO, 7, 5);
}

///
/// @test Transitioning in an exit label generates an error.
///
TEST(StateMachineCompilerErrors, TransitionInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".exit\n"
        "    -> Foo\n"
        "\n"
        "[Foo]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_TR_EXIT, 7, 5);
}

///
/// @test Including an assertion in a state machine generates an error.
///
TEST(StateMachineCompilerErrors, IllegalAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    @assert T == 0\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_ASSERT, 7, 5);
}

///
/// @test Including a stop annotation in a state machine generates an error.
///
TEST(StateMachineCompilerErrors, IllegalStopAnnotation)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Initial]\n"
        ".entry\n"
        "    @stop\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_STOP, 7, 5);
}

///
/// @test Using a state vector element to initialize a local element generates
/// an error.
///
TEST(StateMachineCompilerErrors, LocalElementReferencesStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "I32 foo\n"
        "\n"
        "[local]\n"
        "I32 bar = 1 + foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_LOC_SV_REF, 7, 15);
}

///
/// @test Using a local element to initialize itself generates an error.
///
TEST(StateMachineCompilerErrors, LocalElementReferencesItself)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 bar = 1 + bar\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_SELF_REF, 6, 15);
}

///
/// @test Initializing a local element with an uninitialized local element
/// generates an error.
///
TEST(StateMachineCompilerErrors, LocalElementUseBeforeInitialization)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[local]\n"
        "I32 foo = bar + 1\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_UBI, 6, 11);
}

///
/// @test Passing a null parse to the state machine compiler generates an error.
///
TEST(StateMachineCompilerErrors, NullParse)
{
    const Ref<const StateMachineParse> smParse;
    Ref<const StateMachineAssembly> smAsm;
    CHECK_ERROR(E_SMC_NULL, StateMachineCompiler::compile(smParse,
                                                          nullptr,
                                                          smAsm,
                                                          nullptr));
    CHECK_TRUE(smAsm == nullptr);
}

///
/// @test Naming a state "all_states" generates an error.
///
TEST(StateMachineCompilerErrors, AllStatesSectionNameReserved)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[all_states]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_RSVD, 5, 1);
}

///
/// @test Naming a state "options" generates an error.
///
TEST(StateMachineCompilerErrors, OptionsSectionNameReserved)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[options]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMC_RSVD, 5, 1);
}

///
/// @test Passing an unknown initial state to the compiler generates an error.
///
TEST(StateMachineCompiler, UnknownInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[state_vector]\n"
        "U64 time @alias G\n"
        "U32 state @alias S\n"
        "\n"
        "[Foo]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    Ref<const StateMachineAssembly> smAsm;
    CHECK_ERROR(E_SMC_INIT, StateMachineCompiler::compile(smParse,
                                                          svAsm,
                                                          smAsm,
                                                          nullptr,
                                                          "Bar"));
}
