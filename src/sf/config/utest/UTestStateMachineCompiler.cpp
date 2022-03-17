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

#define COMPILE_SM(kSrc)                                                       \
    std::stringstream smSrc(kSrc);                                             \
    std::shared_ptr<StateMachineCompiler::Assembly> smAsm;                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(smSrc, sv, smAsm, nullptr));

TEST_GROUP(StateMachineCompiler)
{
};

TEST(StateMachineCompiler, Doop)
{
    INIT_SV(
        "[Foo]\n"
        "I32 foo\n"
        "U64 bar\n");
    COMPILE_SM(
        "[STATE_VECTOR]\n"
        "I32 foo\n"
        "U64 bar @ALIAS=G\n"
        "\n"
        "[LOCAL]\n"
        "I32 baz = 0\n"
        "\n"
        "[Initial]\n");
}
