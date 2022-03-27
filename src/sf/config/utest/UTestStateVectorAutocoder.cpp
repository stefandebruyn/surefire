#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/pal/Console.hpp" // rm later
#include "sf/utest/UTest.hpp"

#define PARSE_SV(kSrc)                                                         \
    TOKENIZE(kSrc);                                                            \
    Ref<const StateVectorParse> svParse;                                       \
    CHECK_SUCCESS(StateVectorParse::parse(toks, svParse, nullptr));

TEST_GROUP(StateVectorAutocoder)
{
};

TEST(StateVectorAutocoder, Doop)
{
    PARSE_SV(
        "[Foo]\n"
        "I32 Foo\n"
        "F64 Bar\n"
        "BOOL Baz\n"
        "\n"
        "[Bar]\n"
        "I32 Qux\n"
        "F32 Corge\n");
    std::cout << "\n\n\n" << Console::yellow;
    CHECK_SUCCESS(StateVectorAutocoder::code(std::cout,
                                             "HalcyonStateVector",
                                             svParse,
                                             nullptr));
    std::cout << Console::reset << "\n\n";
}
