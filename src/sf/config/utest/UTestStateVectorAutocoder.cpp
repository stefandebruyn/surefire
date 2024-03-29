////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///                             ---------------
/// @file  sf/config/utest/UTestStateVectorAutocoder.hpp
/// @brief Unit tests for StateVectorAutocoder.
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <cstdlib>

#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/utest/UTest.hpp"

using namespace Sf;

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief String literal path to directory containing state vector harness.
/// 
/// @remark SF_REPO_PATH and PATH_SEP are set by the CMake project.
///
#define HARNESS_PATH                                                           \
    SF_REPO_PATH PATH_SEP "src" PATH_SEP "sf" PATH_SEP "config" PATH_SEP       \
    "utest" PATH_SEP "utest-sv-autocoder-harness"

///
/// @brief String literal path to harness output file.
///
#define HARNESS_OUT_PATH HARNESS_PATH PATH_SEP "out.tmp"

///
/// @brief String literal path to harness executable.
///
#define HARNESS_BIN_PATH HARNESS_PATH PATH_SEP "a.out"

///
/// @brief String literal path to state vector autocode generated for the
/// harness.
///
#define AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateVector.hpp"

///
/// @brief Sets up a test by compiling the state vector and generating autocode
/// on disk.
///
/// @param[in] kSrc  State vector config as string.
///
#define SETUP(kSrc)                                                            \
    /* Compile state vector. */                                                \
    std::stringstream ss(kSrc);                                                \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(ss, svAsm, nullptr));           \
                                                                               \
    /* Generate autocode. */                                                   \
    std::ofstream ofs(AUTOCODE_PATH, std::fstream::out);                       \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(StateVectorAutocoder::code(ofs, "FooStateVector", svAsm));   \
    ofs.close();

///
/// @brief Runs the harness executable, redirects its stdout to a file on disk,
/// disk, and loads the file contents into a string stream SETUP should have
/// been called prior.
///
/// @param[in] kArgs  Harness command line arguments.
///
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

///
/// @brief Unit tests for StateVectorAutocoder.
///
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

///
/// @test All element types are autocoded correctly.
///
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
        "bool k\n");
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
        "bool k\n"
        "Foo 43\n",
        hout.str());
}

///
/// @test A small state vector is autocoded correctly.
///
TEST(StateVectorAutocoder, SmallStateVector)
{
    SETUP(
        "[Foo]\n"
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n"
        "\n"
        "[Bar]\n"
        "I32 qux\n"
        "F32 corge\n");
    RUN_HARNESS("foo bar baz qux corge .Foo .Bar");
    CHECK_EQUAL(
        "I32 foo\n"
        "F64 bar\n"
        "bool baz\n"
        "I32 qux\n"
        "F32 corge\n"
        "Foo 13\n"
        "Bar 8\n",
        hout.str());
}

///
/// @test A (relatively) large) state vector is autocoded correctly.
///
TEST(StateVectorAutocoder, LargeStateVector)
{
    SETUP(
        "[A]\n"
        "I32 a1\n"
        "F64 a2\n"
        "bool a3\n"
        "F64 a4\n"
        "F64 a5\n"
        "F64 a6\n"
        "F64 a7\n"
        "I32 a8\n"
        "I32 a9\n"
        "I32 a10\n"
        "bool a11\n"
        "bool a12\n"
        "bool a13\n"
        "bool a14\n"
        "bool a15\n"
        "bool a16\n"
        "bool a17\n"
        "bool a18\n"
        "bool a19\n"
        "bool a20\n"
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
        "bool b1\n"
        "bool b2\n"
        "bool b3\n"
        "bool b4\n"
        "bool b5\n"
        "bool b6\n"
        "bool b7\n"
        "bool b8\n"
        "bool b9\n"
        "bool b10\n"
        "bool b11\n"
        "bool b12\n"
        "bool b13\n"
        "bool b14\n"
        "bool b15\n"
        "bool b16\n"
        "bool b17\n"
        "bool b18\n"
        "bool b19\n"
        "bool b20\n"
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
        "bool a3\n"
        "F64 a4\n"
        "F64 a5\n"
        "F64 a6\n"
        "F64 a7\n"
        "I32 a8\n"
        "I32 a9\n"
        "I32 a10\n"
        "bool a11\n"
        "bool a12\n"
        "bool a13\n"
        "bool a14\n"
        "bool a15\n"
        "bool a16\n"
        "bool a17\n"
        "bool a18\n"
        "bool a19\n"
        "bool a20\n"
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
        "bool b1\n"
        "bool b2\n"
        "bool b3\n"
        "bool b4\n"
        "bool b5\n"
        "bool b6\n"
        "bool b7\n"
        "bool b8\n"
        "bool b9\n"
        "bool b10\n"
        "bool b11\n"
        "bool b12\n"
        "bool b13\n"
        "bool b14\n"
        "bool b15\n"
        "bool b16\n"
        "bool b17\n"
        "bool b18\n"
        "bool b19\n"
        "bool b20\n"
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

///
/// @test Passing a null state vector assembly to the autocoder returns an
/// error.
///
TEST(StateVectorAutocoder, ErrorNullStateVectorAssembly)
{
    std::stringstream ss;
    CHECK_ERROR(E_SVA_NULL, StateVectorAutocoder::code(ss, "foo", nullptr));
}
