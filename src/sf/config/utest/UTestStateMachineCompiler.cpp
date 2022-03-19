#include <sstream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

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

#define CHECK_LOCAL_ELEM(kElemName, kElemType, kExpectVal)                     \
{                                                                              \
    StateVector& _localSv = smAsm->localStateVector();                         \
    Element<kElemType>* _elem = nullptr;                                       \
    CHECK_SUCCESS(_localSv.getElement(kElemName, _elem));                      \
    CHECK_EQUAL(kExpectVal, _elem->read());                                    \
}

static void checkCompileError(const StateMachineParser::Parse& kParse,
                              StateVector& kSv,
                              const Result kRes,
                              const I32 kLineNum,
                              const I32 kColNum)
{
    // Got expected return code from compiler.
    std::shared_ptr<StateMachineCompiler::Assembly> smAsm;
    ErrorInfo err;
    CHECK_ERROR(kRes, StateMachineCompiler::compile(kParse, kSv, smAsm, &err));

    // Correct line and column numbers of error are identified.
    CHECK_EQUAL(kLineNum, err.lineNum);
    CHECK_EQUAL(kColNum, err.colNum);

    // An error message was given.
    CHECK_TRUE(err.text.size() > 0);
    CHECK_TRUE(err.subtext.size() > 0);

    // Assembly pointer was not populated.
    CHECK_TRUE(smAsm == nullptr);
}

///////////////////////////////// Usage Tests //////////////////////////////////

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
        "U64 time @ALIAS=G \n"
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
    CHECK_LOCAL_ELEM("foo", I32, 1);
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
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

TEST(StateMachineCompiler, TransitionInEntryDoExitLabel)
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
    CHECK_LOCAL_ELEM("foo", I32, 1);

    // After stepping again, `foo` is set to 2 in the state `Foo` entry label.
    SET_SV_ELEM("time", U64, 1);
    CHECK_SUCCESS(sm.step());
    CHECK_LOCAL_ELEM("foo", I32, 2);
}

TEST(StateMachineCompiler, TransitionInStepDoExitLabel)
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

TEST(StateMachineCompiler, GuardTakeIfBranch)
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

TEST(StateMachineCompiler, GuardTakeElseBranch)
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

TEST(StateMachineCompiler, UseAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo @ALIAS=bar\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    bar = 1\n",
        "state",
        1);

    // Element `foo` is set to 1 in the initial state entry label.
    CHECK_SUCCESS(sm.step());
    CHECK_SV_ELEM("foo", I32, 1);
}

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
        "BOOL k\n"
        "I32 foo\n");
    INIT_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
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

TEST(StateMachineCompiler, SpecialElements)
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

///////////////////////////////// Error Tests //////////////////////////////////

TEST_GROUP(StateMachineCompilerErrors)
{
};

TEST(StateMachineCompilerErrors, UnknownStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 bar\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_SV_ELEM, 2, 5);
}

TEST(StateMachineCompilerErrors, UnknownStateVectorElementType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I33 foo\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_TYPE, 2, 1);
}

TEST(StateMachineCompilerErrors, StateVectorElementTypeMismatch)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "F32 foo\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_TYPE_MISM, 2, 1);
}

TEST(StateMachineCompilerErrors, StateVectorElementListedTwice)
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
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_DUPE, 3, 5);
}

TEST(StateMachineCompilerErrors, GlobalTimeElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=G\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_G_TYPE, 2, 5);
}

TEST(StateMachineCompilerErrors, StateElementWrongType)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "I32 foo @ALIAS=S\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_S_TYPE, 2, 5);
}

TEST(StateMachineCompilerErrors, LocalElementReusesSvElementName)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[LOCAL]\n"
        "F64 foo = 0\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_DUPE, 7, 5);
}

TEST(StateMachineCompilerErrors, AssignmentActionUnknownElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 7, 5);
}

TEST(StateMachineCompilerErrors, ErrorInAssignmentActionExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = bar\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_EXC_ELEM, 8, 11);
}

TEST(StateMachineCompilerErrors, TransitionToUnknownState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    -> Foo\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_STATE, 7, 8);
}

TEST(StateMachineCompilerErrors, ErrorInGuardExpression)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    bar: foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_EXC_ELEM, 8, 5);
}

TEST(StateMachineCompilerErrors, ErrorInIfBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: bar = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 8, 15);
}

TEST(StateMachineCompilerErrors, ErrorInElseBranch)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: foo = 1\n"
        "    ELSE: bar = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 9, 11);
}

TEST(StateMachineCompilerErrors, ErrorInNextBlock)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo == 0: foo = 1\n"
        "    ELSE: foo = 2\n"
        "    bar = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 10, 5);
}

TEST(StateMachineCompilerErrors, ErrorInStepLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".STEP\n"
        "    foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 7, 5);
}

TEST(StateMachineCompilerErrors, ErrorInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".EXIT\n"
        "    foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 7, 5);
}

TEST(StateMachineCompilerErrors, ErrorInNonInitialState)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
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
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ASG_ELEM, 13, 5);
}

TEST(StateMachineCompilerErrors, NoGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_NO_G, -1, -1);
}

TEST(StateMachineCompilerErrors, NoStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "\n"
        "[Initial]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_NO_S, -1, -1);
}

TEST(StateMachineCompilerErrors, WriteReadOnlyStateVectorElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo @READ_ONLY\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 8, 5);
}

TEST(StateMachineCompilerErrors, WriteReadOnlyStateVectorElementAlias)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "I32 foo\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "I32 foo @READ_ONLY @ALIAS=bar\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    bar = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 8, 5);
}

TEST(StateMachineCompilerErrors, WriteReadOnlyLocalElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0 @READ_ONLY\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    foo = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 10, 5);
}

TEST(StateMachineCompilerErrors, WriteGlobalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    G = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 7, 5);
}

TEST(StateMachineCompilerErrors, WriteLocalTimeElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    T = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 7, 5);
}

TEST(StateMachineCompilerErrors, WriteStateElement)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".ENTRY\n"
        "    S = 1\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_ELEM_RO, 7, 5);
}

TEST(StateMachineCompilerErrors, TransitionInExitLabel)
{
    INIT_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n");
    TOKENIZE(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS=G\n"
        "U32 state @ALIAS=S\n"
        "\n"
        "[Initial]\n"
        ".EXIT\n"
        "    -> Foo\n"
        "\n"
        "[Foo]\n");
    StateMachineParser::Parse smParse;
    CHECK_SUCCESS(StateMachineParser::parse(toks, smParse, nullptr));
    checkCompileError(smParse, sv, E_SMC_TR_EXIT, 7, 8);
}