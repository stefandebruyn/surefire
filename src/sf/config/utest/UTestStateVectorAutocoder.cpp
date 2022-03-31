#include <fstream>
#include <cstdlib>

#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define HARNESS_PATH                                                           \
    SF_REPO_PATH PATH_SEP "src" PATH_SEP "sf" PATH_SEP "config" PATH_SEP       \
    "utest" PATH_SEP "utest-sv-autocoder-harness"

#define HARNESS_OUT_PATH HARNESS_PATH PATH_SEP "out.tmp"

#define HARNESS_BIN_PATH HARNESS_PATH PATH_SEP "a.out"

#define AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateVector.hpp"

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
    /* Build and run harness. */                                               \
    const I32 status = std::system(                                            \
        "cd " HARNESS_PATH " && make && ./a.out > " HARNESS_OUT_PATH " "       \
        kArgs);                                                                \
    (void) status;                                                             \
                                                                               \
    /* Read harness output into a string stream. */                            \
    std::ifstream houtIfs(HARNESS_OUT_PATH);                                   \
    std::stringstream hout;                                                    \
    hout << houtIfs.rdbuf();

//////////////////////////////////// Tests /////////////////////////////////////

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
        std::remove(AUTOCODE_PATH);
        std::remove(HARNESS_BIN_PATH);
        std::remove(HARNESS_OUT_PATH);
    }
};

TEST(StateVectorAutocoder, AllElementTypes)
{
    SETUP(
        "[Foo]\n"
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
        "BOOL k\n");
    RUN_HARNESS("a b c d e f g h i j k .Foo");
    CHECK_EQUAL(
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
        "Foo 43\n",
        hout.str());
}

TEST(StateVectorAutocoder, SmallStateVector)
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

TEST(StateVectorAutocoder, LargeStateVector)
{
    SETUP(
        "[A]\n"
        "I32 a1\n"
        "F64 a2\n"
        "BOOL a3\n"
        "F64 a4\n"
        "F64 a5\n"
        "F64 a6\n"
        "F64 a7\n"
        "I32 a8\n"
        "I32 a9\n"
        "I32 a10\n"
        "BOOL a11\n"
        "BOOL a12\n"
        "BOOL a13\n"
        "BOOL a14\n"
        "BOOL a15\n"
        "BOOL a16\n"
        "BOOL a17\n"
        "BOOL a18\n"
        "BOOL a19\n"
        "BOOL a20\n"
        "U16 a21\n"
        "U16 a22\n"
        "U16 a23\n"
        "U16 a24\n"
        "F64 a25\n"
        "F64 a26\n"
        "F64 a27\n"
        "F64 a28\n"
        "F64 a29\n"
        "F64 a30\n"
        "F64 a31\n"
        "F64 a32\n"
        "\n"
        "[B]\n"
        "BOOL b1\n"
        "BOOL b2\n"
        "BOOL b3\n"
        "BOOL b4\n"
        "BOOL b5\n"
        "BOOL b6\n"
        "BOOL b7\n"
        "BOOL b8\n"
        "BOOL b9\n"
        "BOOL b10\n"
        "BOOL b11\n"
        "BOOL b12\n"
        "BOOL b13\n"
        "BOOL b14\n"
        "BOOL b15\n"
        "BOOL b16\n"
        "BOOL b17\n"
        "BOOL b18\n"
        "BOOL b19\n"
        "BOOL b20\n"
        "\n"
        "[C]\n"
        "F64 c1\n"
        "F64 c2\n"
        "F64 c3\n"
        "F64 c4\n"
        "F64 c5\n"
        "F64 c6\n"
        "F64 c7\n"
        "F64 c8\n"
        "F64 c9\n"
        "F64 c10\n"
        "F64 c11\n"
        "F64 c12\n"
        "F64 c13\n"
        "F64 c14\n"
        "F64 c15\n"
        "F64 c16\n"
        "F64 c17\n"
        "F64 c18\n"
        "F64 c19\n"
        "F64 c20\n");
    RUN_HARNESS(
        "a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 a13 a14 a15 a16 a17 a18 a19 "
        "a20 a21 a22 a23 a24 a25 a26 a27 a28 a29 a30 a31 a32 b1 b2 b3 b4 b5 b6 "
        "b7 b8 b9 b10 b11 b12 b13 b14 b15 b16 b17 b18 b19 b20 c1 c2 c3 c4 c5 "
        "c6 c7 c8 c9 c10 c11 c12 c13 c14 c15 c16 c17 c18 c19 c20 .A .B .C");
    CHECK_EQUAL(
        "I32 a1\n"
        "F64 a2\n"
        "BOOL a3\n"
        "F64 a4\n"
        "F64 a5\n"
        "F64 a6\n"
        "F64 a7\n"
        "I32 a8\n"
        "I32 a9\n"
        "I32 a10\n"
        "BOOL a11\n"
        "BOOL a12\n"
        "BOOL a13\n"
        "BOOL a14\n"
        "BOOL a15\n"
        "BOOL a16\n"
        "BOOL a17\n"
        "BOOL a18\n"
        "BOOL a19\n"
        "BOOL a20\n"
        "U16 a21\n"
        "U16 a22\n"
        "U16 a23\n"
        "U16 a24\n"
        "F64 a25\n"
        "F64 a26\n"
        "F64 a27\n"
        "F64 a28\n"
        "F64 a29\n"
        "F64 a30\n"
        "F64 a31\n"
        "F64 a32\n"
        "BOOL b1\n"
        "BOOL b2\n"
        "BOOL b3\n"
        "BOOL b4\n"
        "BOOL b5\n"
        "BOOL b6\n"
        "BOOL b7\n"
        "BOOL b8\n"
        "BOOL b9\n"
        "BOOL b10\n"
        "BOOL b11\n"
        "BOOL b12\n"
        "BOOL b13\n"
        "BOOL b14\n"
        "BOOL b15\n"
        "BOOL b16\n"
        "BOOL b17\n"
        "BOOL b18\n"
        "BOOL b19\n"
        "BOOL b20\n"
        "F64 c1\n"
        "F64 c2\n"
        "F64 c3\n"
        "F64 c4\n"
        "F64 c5\n"
        "F64 c6\n"
        "F64 c7\n"
        "F64 c8\n"
        "F64 c9\n"
        "F64 c10\n"
        "F64 c11\n"
        "F64 c12\n"
        "F64 c13\n"
        "F64 c14\n"
        "F64 c15\n"
        "F64 c16\n"
        "F64 c17\n"
        "F64 c18\n"
        "F64 c19\n"
        "F64 c20\n"
        "A 139\n"
        "B 20\n"
        "C 160\n",
        hout.str());
}

TEST(StateVectorAutocoder, CompileError)
{
    // Parse state vector config.
    TOKENIZE(
        "[Foo]\n"
        "I32 foo\n"
        "F64 foo\n");
    Ref<const StateVectorParse> svParse;
    CHECK_SUCCESS(StateVectorParse::parse(toks, svParse, nullptr));

    // Autocoder returns expected compile error.
    std::stringstream ss;
    ErrorInfo err;
    CHECK_ERROR(E_SVA_ELEM_DUPE,
                StateVectorAutocoder::code(ss,
                                           "FooStateVector",
                                           svParse,
                                           &err));

    // Check that autocode output stream was not populated.
    ss.seekg(0, std::ios::end);
    CHECK_EQUAL(0, ss.tellg());
}
