#include <sstream>

#include "sf/config/StateMachineCompiler.hpp"
#include "sf/config/StateVectorCompiler.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define INIT_SV(kSrc)                                                          \
    /* Compile state vector. */                                                \
    std::stringstream svSrc(kSrc);                                             \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(svSrc, svAsm, nullptr));        \
                                                                               \
    /* Get state vector. */                                                    \
    StateVector& sv = svAsm->get();                                            \
    (void) sv;

#define INIT_SM(kSrc)                                                          \
    /* Compile state machine. */                                               \
    std::stringstream smSrc(kSrc);                                             \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(smSrc, svAsm, smAsm, nullptr));\
                                                                               \
    /* Get state machine. */                                                   \
    StateMachine& sm = smAsm->get();                                           \
    (void) sm;

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

TEST_GROUP(StateMachineCompilerErrors)
{
};

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

TEST(StateMachineCompilerErrors, ConfigSectionNameReserved)
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
