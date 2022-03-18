#include <sstream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

#define INIT_SV(kSrc)                                                          \
    std::stringstream svSrc(kSrc);                                             \
    std::shared_ptr<StateVectorCompiler::Assembly> svAsm;                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(svSrc, svAsm, nullptr));        \
    StateVector sv;                                                            \
    CHECK_SUCCESS(StateVector::create(svAsm->getConfig(), sv));

#define INIT_SM(kSrc, kStateElemName, kInitState)                              \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    std::shared_ptr<StateMachineCompiler::Assembly> smAsm;                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(smSrc, sv, smAsm, nullptr));   \
                                                                               \
    /* Set initial state. */                                                   \
    Element<U32>* elemState = nullptr;                                         \
    CHECK_SUCCESS(sv.getElement(kStateElemName, elemState));                   \
    elemState->write(kInitState);                                              \
                                                                               \
    /* Initialize state machine. */                                            \
    StateMachine sm;                                                           \
    CHECK_SUCCESS(StateMachine::create(smAsm->config(), sm));

#define SET_SV_ELEM(kElemName, kElemType, kSetVal)                             \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv.getElement(kElemName, _elem));                            \
    _elem->write(kSetVal);                                                     \
}

#define CHECK_SV_ELEM(kElemName, kElemType, kExpectVal)                        \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv.getElement(kElemName, _elem));                            \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

#define CHECK_LOCAL_SV_ELEM(kElemName, kElemType, kExpectVal)                  \
{                                                                              \
    StateVector& _localSv = smAsm->localStateVector();                         \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(_localSv.getElement(kElemName, _elem));                      \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

TEST_GROUP(StateMachineCompiler)
{
};

TEST(StateMachineCompiler, EntryLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n",
        "state",
        1);

    // Element `foo` is set to 1 in the initial state entry label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_LOCAL_SV_ELEM("foo", I32, 1);
}

TEST(StateMachineCompiler, StepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = foo + 1\n",
        "state",
        1);

    // Element `foo` is incremented twice in the initial state step label.
    CHECK_SUCCESS(sm.step());
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 1);
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_LOCAL_SV_ELEM("foo", I32, 2);
}

TEST(StateMachineCompiler, TransitionAndExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    -> Foo\n"
        ".EXIT\n"
        "    foo = 1\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    foo = 2\n",
        "state",
        1);

    // Element `foo` is set to 1 in the initial state exit label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 0);
    CHECK_SV_ELEM("state", U32, 1);
    CHECK_LOCAL_SV_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("time", U64, 1);
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_LOCAL_SV_ELEM("foo", I32, 2);
}
