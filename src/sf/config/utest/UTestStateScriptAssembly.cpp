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

///////////////////////////////// Usage Tests //////////////////////////////////

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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
        "\n"
        "[LOCAL]\n"
        "U64 sum = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    sum = sum + T\n");
    INIT_SS(
        "[CONFIG]\n"
        "@DELTA_T=3\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
        "U32 state @ALIAS=S\n"
        "U64 time @ALIAS=G\n"
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
        "@DELTA_T=1\n"
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
