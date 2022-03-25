#include <iostream>

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
    const Ref<StateMachine> sm = smAsm->get();

#define INIT_SS(kSrc) \
    std::stringstream ssSrc(kSrc); \
    Vec<Token> toks; \
    ErrorInfo err{}; \
    CHECK_SUCCESS(Tokenizer::tokenize(ssSrc, toks, &err)); \
    Ref<const StateScriptParse> sscrParse; \
    CHECK_SUCCESS(StateScriptParse::parse(toks, sscrParse, nullptr)); \
    Ref<StateScriptAssembly> sscrAsm; \
    CHECK_SUCCESS(StateScriptAssembly::compile(sscrParse, \
                                               smAsm, \
                                               sscrAsm, \
                                               nullptr));

TEST_GROUP(StateScriptAssembly)
{
};

TEST(StateScriptAssembly, Doop)
{
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
        "    foo: bar = 1\n");
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

    bool pass = false;
    CHECK_SUCCESS(sscrAsm->run(pass, err, std::cout));
    CHECK_TRUE(pass);
}
