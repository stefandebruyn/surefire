#include <sstream>

#include "sf/config/StateMachineAssembly.hpp"
#include "sf/config/StateVectorAssembly.hpp"
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

#define INIT_SM(kSrc, kStateElemName, kInitState)                              \
    /* Set initial state. */                                                   \
    Element<U32>* elemState = nullptr;                                         \
    CHECK_SUCCESS(sv->getElement(kStateElemName, elemState));                  \
    elemState->write(kInitState);                                              \
                                                                               \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineAssembly::compile(smSrc, svAsm, smAsm, nullptr));\
                                                                               \
    /* Get state machine. */                                                   \
    StateMachine& sm = *smAsm->get();

#define SET_SV_ELEM(kElemName, kElemType, kSetVal)                             \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv->getElement(kElemName, _elem));                           \
    _elem->write(kSetVal);                                                     \
}

#define CHECK_SV_ELEM(kElemName, kElemType, kExpectVal)                        \
{                                                                              \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(sv->getElement(kElemName, _elem));                           \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

#define CHECK_LOCAL_ELEM(kElemName, kElemType, kExpectVal)                     \
{                                                                              \
    const Ref<StateVector> _localSv = smAsm->localStateVector();               \
    CHECK_TRUE(_localSv != nullptr);                                           \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(_localSv->getElement(kElemName, _elem));                     \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

static void checkCompileError(const Ref<const StateMachineParse> kParse,
                              const Ref<const StateVectorAssembly> kSvAsm,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Got expected return code from compiler.
    Ref<const StateMachineAssembly> smAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineAssembly::compile(kParse,
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
    CHECK_ERROR(kRes, StateMachineAssembly::compile(kParse,
                                                    kSvAsm,
                                                    smAsm,
                                                    nullptr));
}

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateMachineAssembly)
{
};

TEST(StateMachineAssembly, EntryLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G \n"
        "U32 state @ALIAS S\n"
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
    CHECK_LOCAL_ELEM("foo", I32, 1);
}

TEST(StateMachineAssembly, StepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
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
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

TEST(StateMachineAssembly, TransitionInEntryDoExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
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
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

TEST(StateMachineAssembly, TransitionInStepDoExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
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
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("state", U32, 2);
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

TEST(StateMachineAssembly, GuardTakeIfBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    TRUE: foo = 1\n"
        "    ELSE: foo = 2\n"
        "    bar = 1\n",
        "state",
        1);

    // Element `foo` is set to 1 in the if branch.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // Element `bar` is set to 1 regardless of the branch taken.
    CHECK_LOCAL_ELEM("bar", I32, 1);
}

TEST(StateMachineAssembly, GuardTakeElseBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    FALSE: foo = 1\n"
        "    ELSE: foo = 2\n"
        "    bar = 1\n",
        "state",
        1);

    // Element `foo` is set to 2 in the else branch.
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);

    // Element `bar` is set to 1 regardless of the branch taken.
    CHECK_LOCAL_ELEM("bar", I32, 1);
}

TEST(StateMachineAssembly, UseAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n"
        "I32 baz\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo @ALIAS bar\n"
        "I32 baz @ALIAS qux\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    baz = 1\n"
        "    bar = qux\n",
        "state",
        1);

    // Element `foo` is set to 1 in the initial state entry label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 1);
}

TEST(StateMachineAssembly, AllElementTypes)
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
        "BOOL k\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
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
        "BOOL k\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
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
        "BOOL v = 0\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
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
        "    k = TRUE\n"
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
        "    v = TRUE\n"
        ".STEP\n"
        "    foo = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o"
        "          + p + q + r + s + t + u + v\n",
        "state",
        1);

    // Element `foo` is set to 22 in the initial state step label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 22);
}

TEST(StateMachineAssembly, SpecialElements)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    T == 10: -> Foo\n"
        "\n"
        "[Foo]\n",
        "state",
        1);

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

TEST(StateMachineAssembly, StatsFunctionUsingStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    bar = ROLL_AVG(foo, 2)\n",
        "state",
        1);

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

TEST(StateMachineAssembly, TransitionToCurrentState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n"
        "I32 bar\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "I32 bar\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 0\n"
        ".STEP\n"
        "    foo = foo + 1\n"
        "    foo == 3: -> Initial\n"
        ".EXIT\n"
        "    bar = bar + 1\n",
        "state",
        1);

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

TEST(StateMachineAssembly, LocalElementInitialValues)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
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
        "BOOL k = TRUE\n"
        "\n"
        "[Initial]\n",
        "state",
        1);

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

TEST(StateMachineAssembly, InitLocalElemsWithLocalElems)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 1\n"
        "I32 bar = foo + 1\n"
        "I32 baz = bar + 1\n"
        "\n"
        "[Initial]\n",
        "state",
        1);

    CHECK_LOCAL_ELEM("foo", I32, 1);
    CHECK_LOCAL_ELEM("bar", I32, 2);
    CHECK_LOCAL_ELEM("baz", I32, 3);
}

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineAssemblyErrors)
{
};

TEST(StateMachineAssemblyErrors, UnknownStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 bar\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_SV_ELEM, 2, 5);
}

TEST(StateMachineAssemblyErrors, UnknownStateVectorElementType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I33 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_TYPE, 2, 1);
}

TEST(StateMachineAssemblyErrors, StateVectorElementTypeMismatch)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "F32 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_TYPE_MISM, 2, 1);
}

TEST(StateMachineAssemblyErrors, StateVectorElementListedTwice)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_DUPE, 3, 5);
}

TEST(StateMachineAssemblyErrors, GlobalTimeElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS G\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_G_TYPE, 2, 5);
}

TEST(StateMachineAssemblyErrors, StateElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS S\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_S_TYPE, 2, 5);
}

TEST(StateMachineAssemblyErrors, LocalElementReusesSvElementName)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
        "F64 foo = 0\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_DUPE, 7, 5);
}

TEST(StateMachineAssemblyErrors, AssignmentActionUnknownElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 7, 5);
}

TEST(StateMachineAssemblyErrors, ErrorInAssignmentActionExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = bar\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_EXA_ELEM, 8, 11);
}

TEST(StateMachineAssemblyErrors, TransitionToUnknownState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    -> Foo\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_STATE, 7, 8);
}

TEST(StateMachineAssemblyErrors, ErrorInGuardExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    bar: foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_EXA_ELEM, 8, 5);
}

TEST(StateMachineAssemblyErrors, ErrorInIfBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 8, 15);
}

TEST(StateMachineAssemblyErrors, ErrorInElseBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: foo = 1\n"
        "    ELSE: bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 9, 11);
}

TEST(StateMachineAssemblyErrors, ErrorInNextBlock)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: foo = 1\n"
        "    ELSE: foo = 2\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 10, 5);
}

TEST(StateMachineAssemblyErrors, ErrorInStepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 7, 5);
}

TEST(StateMachineAssemblyErrors, ErrorInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".EXIT\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 7, 5);
}

TEST(StateMachineAssemblyErrors, ErrorInNonInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n"
        "    -> Foo\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASG_ELEM, 13, 5);
}

TEST(StateMachineAssemblyErrors, NoGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_NO_G, -1, -1);
}

TEST(StateMachineAssemblyErrors, NoStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_NO_S, -1, -1);
}

TEST(StateMachineAssemblyErrors, WriteReadOnlyStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo @READ_ONLY\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 8, 5);
}

TEST(StateMachineAssemblyErrors, WriteReadOnlyStateVectorElementAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo @READ_ONLY @ALIAS bar\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    bar = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 8, 5);
}

TEST(StateMachineAssemblyErrors, WriteReadOnlyLocalElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 10, 5);
}

TEST(StateMachineAssemblyErrors, WriteGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    G = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 7, 5);
}

TEST(StateMachineAssemblyErrors, WriteLocalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    T = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 7, 5);
}

TEST(StateMachineAssemblyErrors, WriteStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    S = 1\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ELEM_RO, 7, 5);
}

TEST(StateMachineAssemblyErrors, TransitionInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".EXIT\n"
        "    -> Foo\n"
        "\n"
        "[Foo]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_TR_EXIT, 7, 5);
}

TEST(StateMachineAssemblyErrors, IllegalAssert)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    @ASSERT T == 0\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_ASSERT, 7, 5);
}

TEST(StateMachineAssemblyErrors, IllegalStopAnnotation)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    @STOP\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_STOP, 7, 5);
}

TEST(StateMachineAssemblyErrors, LocalElementReferencesStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 1 + foo\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_LOC_SV_REF, 7, 15);
}

TEST(StateMachineAssemblyErrors, LocalElementReferencesItself)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 bar = 1 + bar\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_SELF_REF, 6, 15);
}

TEST(StateMachineAssemblyErrors, LocalElementUseBeforeInitialization)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = bar + 1\n"
        "I32 bar = 0\n"
        "\n"
        "[Initial]\n");
    Ref<const StateMachineParse> smParse;
    CHECK_SUCCESS(StateMachineParse::parse(toks, smParse, nullptr));
    checkCompileError(smParse, svAsm, E_SMA_UBI, 6, 11);
}

TEST(StateMachineAssemblyErrors, NullParse)
{
    const Ref<const StateMachineParse> smParse;
    Ref<const StateMachineAssembly> smAsm;
    CHECK_ERROR(E_SMA_NULL, StateMachineAssembly::compile(smParse,
                                                          nullptr,
                                                          smAsm,
                                                          nullptr));
    CHECK_TRUE(smAsm == nullptr);
}
