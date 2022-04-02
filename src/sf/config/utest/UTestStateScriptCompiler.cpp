#include "sf/config/StateScriptCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define INIT_SV(kSrc)                                                          \
    /* Compile state vector. */                                                \
    std::stringstream svSrc(kSrc);                                             \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(svSrc, svAsm, nullptr));        \
                                                                               \
    /* Get state vector. */                                                    \
    StateVector& sv = svAsm->get();

#define INIT_SM(kSrc)                                                          \
    /* Set initial state. */                                                   \
    Element<U32>* elemState = nullptr;                                         \
    CHECK_SUCCESS(sv.getElement("state", elemState));                          \
    elemState->write(1);                                                       \
                                                                               \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(smSrc, svAsm, smAsm, nullptr));\
                                                                               \
    /* Get state machine. */                                                   \
    StateMachine& sm = smAsm->get();                                           \
                                                                               \
    /* Get local state vector. */                                              \
    StateVector& localSv = smAsm->localStateVector();

#define INIT_SS(kSrc)                                                          \
    std::stringstream ssSrc(kSrc);                                             \
    Ref<StateScriptAssembly> ssAsm;                                            \
    ErrorInfo ssTokInfo{};                                                     \
    CHECK_SUCCESS(StateScriptCompiler::compile(ssSrc,                          \
                                               smAsm,                          \
                                               ssAsm,                          \
                                               &ssTokInfo));

#define CHECK_SV_ELEM(kElemName, kElemType, kExpectVal)                        \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv.getElement(kElemName, _elem));                            \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

#define CHECK_LOCAL_ELEM(kElemName, kElemType, kExpectVal)                     \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(localSv.getElement(kElemName, _elem));                       \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

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

TEST_GROUP(StateScriptCompiler)
{
};

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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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
        "[config]\n"
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

TEST_GROUP(StateScriptCompilerErrors)
{
};

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "[Foo]\n");
    checkCompileError(ss, smAsm, E_SSC_DUPE, 5, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Bar]\n");
    checkCompileError(ss, smAsm, E_SSC_STATE, 4, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@assert foo == 0\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "@stop\n");
    checkCompileError(ss, smAsm, E_SSC_GUARD, 5, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "T == 0: foo = 1\n"
        "else: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSC_ELSE, 6, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "bar == 1: foo = 1\n");
    checkCompileError(ss, smAsm, E_EXC_ELEM, 5, 1);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "foo == 1: T == 0: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSC_NEST, 5, 11);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true {\n"
        "    @stop\n"
        "    foo = 1\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSC_UNRCH, 7, 5);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true {\n"
        "    @stop\n"
        "    @assert foo == 0\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSC_UNRCH, 7, 5);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: @assert bar == 1\n");
    checkCompileError(ss, smAsm, E_EXC_ELEM, 5, 15);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: bar = 1\n");
    checkCompileError(ss, smAsm, E_SMC_ASG_ELEM, 5, 7);
}

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
        "[config]\n"
        "delta_t 1\n"
        "\n"
        "[Foo]\n"
        "true: foo = 1\n");
    checkCompileError(ss, smAsm, E_SSC_STOP, -1, -1);
}

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
        "[config]\n"
        "delta_t 9223372036854775806\n" // I64 max value - 1
        "\n"
        "[Initial]\n"
        "T == 3: @stop\n");

    // Run state script. Expect an error due to global clock overflow.
    StateScriptAssembly::Report report{};
    CHECK_ERROR(E_SSC_OVFL, ssAsm->run(ssTokInfo, report));
}

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
        "[config]\n"
        "delta_t 1.5\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

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
        "[config]\n"
        "delta_t -1\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

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
        "[config]\n"
        "delta_t 999999999999999999999999999999999999999999999999999999999999\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_DT, 2, 9);
}

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
        "[config]\n"
        "delta_t 1\n"
        "init_state Bar\n"
        "\n"
        "[Foo]\n"
        "true: @stop\n");
    checkCompileError(ss, smAsm, E_SSC_STATE, 3, 12);
}
