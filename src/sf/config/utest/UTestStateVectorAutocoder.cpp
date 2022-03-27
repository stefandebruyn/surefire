#include <fstream>
#include <cstdlib>

#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define SETUP(kSrc)                                                            \
    /* Parse state vector config. */                                           \
    TOKENIZE(kSrc);                                                            \
    Ref<const StateVectorParse> svParse;                                       \
    CHECK_SUCCESS(StateVectorParse::parse(toks, svParse, nullptr));            \
                                                                               \
    /* Generate autocode. */                                                   \
    std::ofstream ofs(AUTOCODE_PATH, std::fstream::out);                       \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(                                                             \
        StateVectorAutocoder::code(ofs, "FooStateVector", svParse, nullptr));  \
    ofs.close();

#define RUN_HARNESS(kArgs)                                                     \
    /* CD into harness, build binary, and run. */                              \
    const I32 stat = std::system(                                              \
        "cd " HARNESS_PATH " && make && ./a.out > " HARNESS_OUT_PATH " "       \
        kArgs);                                                                \
    CHECK_EQUAL(EXIT_SUCCESS, stat);                                           \
                                                                               \
    /* Load harness output into a string stream. */                            \
    std::ifstream houtIfs(HARNESS_OUT_PATH);                                   \
    std::stringstream hout;                                                    \
    hout << houtIfs.rdbuf();

#ifdef _WIN32
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif

// TODO
#define CWD "/home/stefan/thesis/src/sf/config/utest"

#define HARNESS_PATH CWD PATH_SEP "utest-sv-autocoder-harness"

#define HARNESS_OUT_PATH HARNESS_PATH PATH_SEP "out.tmp"

#define HARNESS_BIN_PATH HARNESS_PATH PATH_SEP "a.out"

#define AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateVector.hpp"

///////////////////////////// Correct Usage Tests //////////////////////////////

TEST_GROUP(StateVectorAutocoder)
{
    void setup()
    {
        // Print newline so that harness compilation output is easier to read
        // amidst CppUTest output.
        std::cout << "\n";
    }

    void teardown()
    {
        // Delete files that may have been created.
        std::remove(HARNESS_OUT_PATH);
        std::remove(HARNESS_BIN_PATH);
        std::remove(AUTOCODE_PATH);
    }
};

TEST(StateVectorAutocoder, Doop)
{
    SETUP(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "BOOL baz\n"
        "\n"
        "[Bar]\n"
        "I32 qux\n"
        "F32 corge\n");
    RUN_HARNESS("foo bar baz qux corge .Foo .Bar");
    CHECK_EQUAL(
        "I32 foo\n"
        "F64 bar\n"
        "BOOL baz\n"
        "I32 qux\n"
        "F32 corge\n"
        "Foo 13\n"
        "Bar 8\n",
        hout.str());
}
