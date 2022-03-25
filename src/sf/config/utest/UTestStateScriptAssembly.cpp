#include "sf/config/StateScriptAssembly.hpp"
#include "sf/utest/UTest.hpp"

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

TEST_GROUP(StateScriptAssembly)
{
};

TEST(StateScriptAssembly, BasicPass)
{
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

    // Check final state vector.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 0);
}

TEST(StateScriptAssembly, BasicFail)
{
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
        "    @ASSERT bar == 2\n"
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

    // Check final state vector.
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("foo", bool, true);
    CHECK_LOCAL_ELEM("bar", I32, 1);
    CHECK_LOCAL_ELEM("T", U64, 0);
}

TEST(StateScriptAssembly, Stages)
{
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
