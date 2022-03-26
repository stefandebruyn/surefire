#include "sf/config/StateScriptAssembly.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define INIT_SV(kSrc)                                                          \
    /* Compile state vector. */                                                \
    std::stringstream svSrc(kSrc);                                             \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorAssembly::compile(svSrc, svAsm, nullptr));        \
                                                                               \
    /* Get state vector. */                                                    \
    const Ref<StateVector> sv = svAsm->get();

#define INIT_SM(kSrc)                                                          \
    /* Set initial state. */                                                   \
    Element<U32>* elemState = nullptr;                                         \
    CHECK_SUCCESS(sv->getElement("state", elemState));                         \
    elemState->write(1);                                                       \
                                                                               \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineAssembly::compile(smSrc, svAsm, smAsm, nullptr));\
                                                                               \
    /* Get state machine. */                                                   \
    const Ref<StateMachine> sm = smAsm->get();                                 \
                                                                               \
    /* Get local state vector. */                                              \
    const Ref<StateVector> localSv = smAsm->localStateVector();

#define INIT_SS(kSrc)                                                          \
    std::stringstream ssSrc(kSrc);                                             \
    Ref<StateScriptAssembly> ssAsm;                                            \
    ErrorInfo ssTokInfo{};                                                     \
    CHECK_SUCCESS(StateScriptAssembly::compile(ssSrc,                          \
                                               smAsm,                          \
                                               ssAsm,                          \
                                               &ssTokInfo));

#define CHECK_SV_ELEM(kElemName, kElemType, kExpectVal)                        \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv->getElement(kElemName, _elem));                           \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

#define CHECK_LOCAL_ELEM(kElemName, kElemType, kExpectVal)                     \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(localSv->getElement(kElemName, _elem));                      \
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
    CHECK_ERROR(kRes, StateScriptAssembly::compile(kSrc,
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
    CHECK_ERROR(kRes, StateScriptAssembly::compile(ssCpy,
                                                   kSmAsm,
                                                   ssAsm,
                                                   nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateScriptAssembly)
{
};

TEST(StateScriptAssembly, SingleStepPass)
{
    // General logic: state script executes for a single step. The state machine
    // increments element `bar` when element `foo` is true; `foo` is set via
    // a state script input.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo: bar = bar + 1\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 1\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, SingleStepFail)
{
    // General logic: same as in `SingleStepPass`, except state script expects
    // a different value for `bar` that is not met.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo: bar = bar + 1\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 2\n" // Assert fails on T=0
        "    @STOP\n"
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

TEST(StateScriptAssembly, MultiStepPass)
{
    // General logic: element `bar` is updated according to some basic logic
    // that references a state vector element and the current time. State script
    // checks the value of `bar` at each step from T=0 to T=10.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    T < 5 {\n"
        "        foo: bar = bar + 1\n"
        "        ELSE: bar = -1\n"
        "    }\n"
        "    ELSE: bar = bar + 2\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == -1\n"
        "}\n"
        "T == 1 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 0\n"
        "}\n"
        "T == 2: @ASSERT bar == 1\n"
        "T == 3 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == -1\n"
        "}\n"
        "T == 4 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 0\n"
        "}\n"
        "T >= 5 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == 2 * (T - 4)\n"
        "}\n"
        "T == 10: @STOP\n");

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

TEST(StateScriptAssembly, MultiStepFail)
{
    // General logic: same as in `MultiStepPass`, except state machine fails to
    // update element `bar` on T=8.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    T < 5 {\n"
        "        foo: bar = bar + 1\n"
        "        ELSE: bar = -1\n"
        "    }\n"
        "    ELSE: T != 8: bar = bar + 2\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == -1\n"
        "}\n"
        "T == 1 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 0\n"
        "}\n"
        "T == 2: @ASSERT bar == 1\n"
        "T == 3 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == -1\n"
        "}\n"
        "T == 4 {\n"
        "    foo = TRUE\n"
        "    @ASSERT bar == 0\n"
        "}\n"
        "T >= 5 {\n"
        "    foo = FALSE\n"
        "    @ASSERT bar == 2 * (T - 4)\n" // Assert fails on T=8
        "}\n"
        "T == 10: @STOP\n");

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

TEST(StateScriptAssembly, DeltaT)
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
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[LOCAL]\n"
        "U64 sum = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    sum = sum + T\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 3\n"
        "\n"
        "[Initial]\n"
        "T == 9: @STOP\n");

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

TEST(StateScriptAssembly, StateTime)
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
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[LOCAL]\n"
        "BOOL foo = FALSE\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = (T == 1 OR T == 3 OR T == 5)\n"
        "    T == 5: -> Initial\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0: @ASSERT !foo\n"
        "T == 1: @ASSERT foo\n"
        "T == 2: @ASSERT !foo\n"
        "T == 3: @ASSERT foo\n"
        "T == 4: @ASSERT !foo\n"
        "T == 5: @ASSERT foo\n"
        "G == 11: @STOP\n");

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

TEST(StateScriptAssembly, StateTimeFail)
{
    // General logic: same as in `StateTime`, but the state machine sets an
    // incorrect value for element `foo` on T=4 on the 2nd loop of the state.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[LOCAL]\n"
        "BOOL foo = FALSE\n"
        "BOOL looped = FALSE\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = (T == 1 OR T == 3 OR T == 5)\n"
        "    looped AND T == 4: foo = TRUE\n"
        "    T == 5: -> Initial\n"
        ".EXIT\n"
        "    looped = TRUE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 0: @ASSERT !foo\n"
        "T == 1: @ASSERT foo\n"
        "T == 2: @ASSERT !foo\n"
        "T == 3: @ASSERT foo\n"
        "T == 4: @ASSERT !foo\n"
        "T == 5: @ASSERT foo\n"
        "G == 11: @STOP\n");

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

TEST(StateScriptAssembly, MultiState)
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
        "BOOL trans\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL trans\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = 1\n"
        ".STEP\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".EXIT\n"
        "    trans = FALSE\n"
        "\n"
        "[Bar]\n"
        ".ENTRY\n"
        "    bar = foo\n"
        ".STEP\n"
        "    baz = T / 2\n"
        "    trans: -> Foo\n"
        "    bar = bar + 1\n"
        ".EXIT\n"
        "    trans = FALSE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[ALL_STATES]\n"
        "TRUE: @ASSERT baz == T / 2\n"
        "\n"
        "[Foo]\n"
        "T == 0: @ASSERT foo == 2\n"
        "T == 1: @ASSERT foo == 4\n"
        "T == 2: @ASSERT foo == 8\n"
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @ASSERT bar == 9\n"
        "T == 1: @ASSERT bar == 10\n"
        "T == 2: @ASSERT bar == 11\n"
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT bar == 11\n"
        "    @ASSERT foo == 8\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, MultiStateFailInStateSection)
{
    // General logic: same as in `MultiState`, but state `Bar` fails to execute
    // its unique behavior on T=2.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL trans\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL trans\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = 1\n"
        ".STEP\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".EXIT\n"
        "    trans = FALSE\n"
        "\n"
        "[Bar]\n"
        ".ENTRY\n"
        "    bar = foo\n"
        ".STEP\n"
        "    baz = T / 2\n"
        "    trans: -> Foo\n"
        "    T != 2: bar = bar + 1\n"
        ".EXIT\n"
        "    trans = FALSE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[ALL_STATES]\n"
        "TRUE: @ASSERT baz == T / 2\n"
        "\n"
        "[Foo]\n"
        "T == 0: @ASSERT foo == 2\n"
        "T == 1: @ASSERT foo == 4\n"
        "T == 2: @ASSERT foo == 8\n"
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @ASSERT bar == 9\n"
        "T == 1: @ASSERT bar == 10\n"
        "T == 2: @ASSERT bar == 11\n" // Failing assert
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT bar == 11\n"
        "    @ASSERT foo == 8\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, MultiStateFailInAllStatesSection)
{
    // General logic: same as in `MultiState`, but state `Bar` fails to execute
    // its shared behavior on T=2.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL trans\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL trans\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "F64 baz = 0\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = 1\n"
        ".STEP\n"
        "    baz = T / 2\n"
        "    trans: -> Bar\n"
        "    foo = foo * 2\n"
        ".EXIT\n"
        "    trans = FALSE\n"
        "\n"
        "[Bar]\n"
        ".ENTRY\n"
        "    bar = foo\n"
        ".STEP\n"
        "    T != 2: baz = T / 2\n"
        "    trans: -> Foo\n"
        "    bar = bar + 1\n"
        ".EXIT\n"
        "    trans = FALSE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[ALL_STATES]\n"
        "TRUE: @ASSERT baz == T / 2\n" // Failing assert
        "\n"
        "[Foo]\n"
        "T == 0: @ASSERT foo == 2\n"
        "T == 1: @ASSERT foo == 4\n"
        "T == 2: @ASSERT foo == 8\n"
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT foo == 8\n"
        "}\n"
        "\n"
        "[Bar]\n"
        "T == 0: @ASSERT bar == 9\n"
        "T == 1: @ASSERT bar == 10\n"
        "T == 2: @ASSERT bar == 11\n"
        "T == 3 {\n"
        "    trans = TRUE\n"
        "    @ASSERT bar == 11\n"
        "    @ASSERT foo == 8\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, UseAliasInAssert)
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
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo @ALIAS bar\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = foo + 1\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "T == 10 {\n"
        "    @ASSERT bar == 11\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, UseAliasInInput)
{
    // General logic: state `Initial` sets element `bar` to true when `foo` is
    // true. `foo` is aliased to `baz`.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n"
        "BOOL bar\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo @ALIAS baz\n"
        "BOOL bar\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo: bar = TRUE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "TRUE {\n"
        "    baz = TRUE\n"
        "    @ASSERT bar == TRUE\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, UseAliasInGuard)
{
    // General logic: state `Initial` sets element `foo` to true on T=5. `foo`
    // is aliased to `bar`. State script stops when `bar` is true.

    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo @ALIAS bar\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    T == 5: foo = TRUE\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "bar {\n"
        "    @ASSERT T == 6\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, UpdateExpressionStats)
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
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    T == 0: foo = 3\n"
        "    T == 1: foo = 2\n"
        "    T == 2: foo = 1\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Initial]\n"
        "ROLL_MAX(foo, 2) == 2 {\n"
        "    @ASSERT T == 3\n"
        "    @STOP\n"
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

TEST(StateScriptAssembly, ConfigInitialState)
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
        "BOOL foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "BOOL foo\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = TRUE\n"
        "\n"
        "[Bar]\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "INIT_STATE Bar\n"
        "\n"
        "[ALL_STATES]\n"
        "TRUE: @STOP\n");

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

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateScriptAssemblyErrors)
{
};

TEST(StateScriptAssemblyErrors, NullParse)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    Ref<StateScriptAssembly> ssAsm;
    Ref<const StateScriptParse> ssParse;
    CHECK_ERROR(E_SSA_NULL, StateScriptAssembly::compile(ssParse,
                                                         smAsm,
                                                         ssAsm,
                                                         nullptr));
    CHECK_TRUE(ssAsm == nullptr);
}

TEST(StateScriptAssemblyErrors, DupeSection)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "[Foo]\n");
    checkCompileError(ss, smAsm, E_SSA_DUPE, 5, 1);
}

TEST(StateScriptAssemblyErrors, UnknownState)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Bar]\n");
    checkCompileError(ss, smAsm, E_SSA_STATE, 4, 1);
}

TEST(StateScriptAssemblyErrors, UnguardedInput)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "foo = 1\n");
    checkCompileError(ss, smAsm, E_SSA_GUARD, 5, 1);
}

TEST(StateScriptAssemblyErrors, UnguardedAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "@ASSERT foo == 0\n");
    checkCompileError(ss, smAsm, E_SSA_GUARD, 5, 1);
}

TEST(StateScriptAssemblyErrors, UnguardedStop)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "@STOP\n");
    checkCompileError(ss, smAsm, E_SSA_GUARD, 5, 1);
}

TEST(StateScriptAssemblyErrors, IllegalElse)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "T == 0: foo = 1\n"
        "ELSE: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSA_ELSE, 6, 1);
}

TEST(StateScriptAssemblyErrors, SurfaceErrorInGuardExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "bar == 1: foo = 1\n");
    checkCompileError(ss, smAsm, E_EXA_ELEM, 5, 1);
}

TEST(StateScriptAssemblyErrors, NestedGuard)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "foo == 1: T == 0: foo = 2\n");
    checkCompileError(ss, smAsm, E_SSA_NEST, 5, 11);
}

TEST(StateScriptAssemblyErrors, UnreachableInput)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "TRUE {\n"
        "    @STOP\n"
        "    foo = 1\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSA_UNRCH, 7, 5);
}

TEST(StateScriptAssemblyErrors, UnreachableAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "TRUE {\n"
        "    @STOP\n"
        "    @ASSERT foo == 0\n"
        "}\n");
    checkCompileError(ss, smAsm, E_SSA_UNRCH, 7, 5);
}

TEST(StateScriptAssemblyErrors, SurfaceErrorInAssertExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "TRUE: @ASSERT bar == 1\n");
    checkCompileError(ss, smAsm, E_EXA_ELEM, 5, 15);
}

TEST(StateScriptAssemblyErrors, SurfaceErrorInAction)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "TRUE: bar = 1\n");
    checkCompileError(ss, smAsm, E_SMA_ASG_ELEM, 5, 7);
}

TEST(StateScriptAssemblyErrors, NoStop)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "I32 foo\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "\n"
        "[Foo]\n"
        "TRUE: foo = 1\n");
    checkCompileError(ss, smAsm, E_SSA_STOP, -1, -1);
}

TEST(StateScriptAssemblyErrors, GlobalClockOverflow)
{
    // Compile objects.
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Initial]\n");
    INIT_SS(
        "[CONFIG]\n"
        "DELTA_T 9223372036854775806\n" // I64 max value - 1
        "\n"
        "[Initial]\n"
        "T == 3: @STOP\n");

    // Run state script. Expect an error due to global clock overflow.
    StateScriptAssembly::Report report{};
    CHECK_ERROR(E_SSA_OVFL, ssAsm->run(ssTokInfo, report));
}

TEST(StateScriptAssemblyErrors, DeltaTFloating)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1.5\n"
        "\n"
        "[Foo]\n"
        "TRUE: @STOP\n");
    checkCompileError(ss, smAsm, E_SSA_DT, 2, 9);
}

TEST(StateScriptAssemblyErrors, DeltaTNegative)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T -1\n"
        "\n"
        "[Foo]\n"
        "TRUE: @STOP\n");
    checkCompileError(ss, smAsm, E_SSA_DT, 2, 9);
}

TEST(StateScriptAssemblyErrors, DeltaTTooLarge)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 999999999999999999999999999999999999999999999999999999999999\n"
        "\n"
        "[Foo]\n"
        "TRUE: @STOP\n");
    checkCompileError(ss, smAsm, E_SSA_DT, 2, 9);
}

TEST(StateScriptAssemblyErrors, UnknownInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U32 state\n"
        "U64 time\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Foo]\n");
    std::stringstream ss(
        "[CONFIG]\n"
        "DELTA_T 1\n"
        "INIT_STATE Bar\n"
        "\n"
        "[Foo]\n"
        "TRUE: @STOP\n");
    checkCompileError(ss, smAsm, E_SSA_STATE, 3, 12);
}
