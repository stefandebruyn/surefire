#include <fstream>
#include <sstream>

#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(StateMachineAutocoder)
{
};

TEST(StateMachineAutocoder, Doop)
{
    std::stringstream svSrc(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "BOOL qux\n");
    Ref<const StateVectorAssembly> svAsm;
    CHECK_SUCCESS(StateVectorAssembly::compile(svSrc, svAsm, nullptr));

    std::stringstream smSrc(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "BOOL qux @ALIAS corge"
        "\n"
        "[LOCAL]\n"
        "I32 foo = 0\n"
        "F64 bar = 0\n"
        "BOOL baz = 0\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    bar + 100 < ROLL_AVG(foo, 10) OR baz: foo = 1\n"
        "    corge = TRUE\n"
        ".STEP\n"
        "    G == 10: -> Bar\n"
        "\n"
        "[Bar]\n");
    Ref<const StateMachineAssembly> smAsm;
    CHECK_SUCCESS(StateMachineAssembly::compile(smSrc, svAsm, smAsm, nullptr));

    std::ofstream ofs("/home/stefan/thesis/src/sf/config/utest/utest-sm-autocoder-harness/FooStateMachine.hpp");
    CHECK_SUCCESS(StateMachineAutocoder::code(ofs, "FooStateMachine", smAsm));
}
