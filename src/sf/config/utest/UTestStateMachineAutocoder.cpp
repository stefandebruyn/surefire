////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/config/utest/UTestStateMachineAutocoder.hpp
/// @brief Unit tests for StateMachineAutocoder.
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/utest/UTest.hpp"
#include "utest-sm-autocoder-harness/Common.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief String literal path to directory containing state machine harness.
/// 
/// @remark SF_REPO_PATH and PATH_SEP are set by the CMake project.
///
#define HARNESS_PATH                                                           \
    SF_REPO_PATH PATH_SEP "src" PATH_SEP "sf" PATH_SEP "config" PATH_SEP       \
    "utest" PATH_SEP "utest-sm-autocoder-harness"

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
#define SV_AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateVector.hpp"

///
/// @brief String literal path to state machine autocoded generated for the
/// harness.
///
#define SM_AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateMachine.hpp"

///
/// @brief Compiles state machine and generates autocode for the harness on
/// disk. Element values in the compiled state machine are randomized except for
/// the global time, which is initially 1. The global time element must be
/// named "time".
///
/// @param[in] kPath  Path to state vector config.
///
#define AUTOCODE_SV(kPath)                                                     \
    /* Compile state vector. */                                                \
    String path = kPath;                                                       \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorCompiler::compile(path, svAsm, nullptr));         \
                                                                               \
    /* Randomize state vector element values using the same function as the    \
       harness. */                                                             \
    resetRandomGenerator();                                                    \
    randomizeStateVector(svAsm->config());                                     \
                                                                               \
    /* Set initial global time, which we don't want randomized, back to 0. */  \
    Element<U64>* elemGlobalTime = nullptr;                                    \
    CHECK_SUCCESS(svAsm->get().getElement("time", elemGlobalTime));            \
    elemGlobalTime->write(0);                                                  \
                                                                               \
    /* Generate state vector autocode. */                                      \
    std::ofstream ofs(SV_AUTOCODE_PATH, std::fstream::out);                    \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(StateVectorAutocoder::code(ofs, "FooStateVector", svAsm));   \
    ofs.close();

///
/// @brief Generates harness state machine autocode on disk. AUTOCODE_SV should
/// have been called prior.
///
/// @param[in] kPath  Path to state machine config.
///
#define AUTOCODE_SM(kPath)                                                     \
    /* Compile state machine. */                                               \
    path = kPath;                                                              \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineCompiler::compile(path, svAsm, smAsm, nullptr)); \
                                                                               \
    /* Generate state machine autocode. */                                     \
    ofs.open(SM_AUTOCODE_PATH, std::fstream::out);                             \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(                                                             \
        StateMachineAutocoder::code(ofs, "FooStateMachine", smAsm));           \
    ofs.close();

///
/// @brief Runs the harness executable, redirects its stdout to a file on disk,
/// disk, and loads the file contents into a string stream AUTOCODE_SM should
/// have been called prior.
///
/// @param[in] kArgs  Harness command line arguments.
///
#define RUN_HARNESS(kArgs)                                                     \
    /* Build and run harness. */                                               \
    const I32 status = std::system(                                            \
        "cd " HARNESS_PATH " && make && ./a.out " kArgs " > "                  \
        HARNESS_OUT_PATH);                                                     \
    (void) status;                                                             \
                                                                               \
    /* Read harness output into a string stream. */                            \
    std::ifstream houtIfs(HARNESS_OUT_PATH);                                   \
    std::stringstream hout;                                                    \
    hout << houtIfs.rdbuf();

///
/// @brief Runs the state machine previously compiled in-memory and compares
/// its output to the harness output. RUN_HARNESS should have been called prior.
///
/// @param[in] kSmSteps  Number of state machine steps to run for.
///
#define CHECK_HARNESS_OUT(kSmSteps)                                            \
    std::stringstream expectOut;                                               \
    /* Fix floating output precision for consistent output. The precision      \
       should match the precision used by the harness. */                      \
    expectOut << std::setprecision(std::numeric_limits<F64>::digits10);        \
    runStateMachine(svAsm, smAsm, kSmSteps, expectOut);                        \
    CHECK_EQUAL(expectOut.str(), hout.str());

#define SET_SV_ELEM(kElemName, kType, kVal)                                    \
{                                                                              \
    Element<kType>* _elem = nullptr;                                           \
    CHECK_SUCCESS(svAsm->get().getElement(kElemName, _elem));                  \
    _elem->write(kVal);                                                        \
}

///
/// @brief Runs a state machine compiled in-memory in the exact same way the
/// harness runs its autocoded state machine. The state vector is printed to
/// a specified output stream each step in the exact same format used by the
/// harness.
///
/// @param[in]  kSvAsm    State vector assembly used by state machine.
/// @param[in]  kSmAsm    State machine assembly to run.
/// @param[in]  kSmSteps  Number of state machine steps to run for.
/// @param[out] kOs       Output stream to print state vector to.
///
static void runStateMachine(const Ref<const StateVectorAssembly> kSvAsm,
                            const Ref<const StateMachineAssembly> kSmAsm,
                            const U32 kSmSteps,
                            std::ostream& kOs)
{
    StateVector& sv = kSvAsm->get();
    Element<U64>* elemGlobalTime = nullptr;
    CHECK_SUCCESS(sv.getElement("time", elemGlobalTime));

    StateMachine& sm = kSmAsm->get();
    for (U32 i = 0; i < kSmSteps; ++i)
    {
        // Increment global time. Modulate the increment to test state machine
        // behavior with varying delta T.
        const U64 deltaT = ((i % 3) + 1);
        elemGlobalTime->write(elemGlobalTime->read() + deltaT);

        // Step state machine.
        CHECK_SUCCESS(sm.step());

        // Print state vector.
        kOs << "---- STEP " << i << " ----\n";
        printStateVector(kSvAsm->config(), kOs);
    }
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Tests which autocode state machines and compare their behavior to
/// state machines compiled in-memory, which are known correct from other tests.
///
/// @note Preconditions for tests in group:
///
///   1. The global time element is named "time"
///   2. The state element is named "state"
///   3. The test execution host recognizes the commands "g++" and "make", and
///      supports output redirection with ">"
///
TEST_GROUP(StateMachineAutocoder)
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
        std::remove(SV_AUTOCODE_PATH);
        std::remove(SM_AUTOCODE_PATH);
        std::remove(HARNESS_BIN_PATH);
        std::remove(HARNESS_OUT_PATH);
    }
};

///
/// @test Autocoded state machine with a bunch of random, complex logic meant to
/// exercise the full range of language syntax. Each state machine step, each
/// local element gets copied into a corresponding state vector element so that
/// local elements are visible to the harness.
///
TEST(StateMachineAutocoder, Nonsense)
{
    AUTOCODE_SV(HARNESS_PATH PATH_SEP "configs" PATH_SEP "nonsense.sv");
    AUTOCODE_SM(HARNESS_PATH PATH_SEP "configs" PATH_SEP "nonsense.sm");
    RUN_HARNESS("1000");
    CHECK_HARNESS_OUT(1000);
}

///
/// @test Autocoded state machine that computes Fibonacci numbers.
///
TEST(StateMachineAutocoder, Fib)
{
    AUTOCODE_SV(HARNESS_PATH PATH_SEP "configs" PATH_SEP "fib.sv");
    AUTOCODE_SM(HARNESS_PATH PATH_SEP "configs" PATH_SEP "fib.sm");
    RUN_HARNESS("50 n=50");
    SET_SV_ELEM("n", U64, 50);
    CHECK_HARNESS_OUT(50);
}

///
/// @test Autocoded state machine that demonstrates safe type conversion for all
/// types.
///
TEST(StateMachineAutocoder, SafeConversion)
{
    AUTOCODE_SV(HARNESS_PATH PATH_SEP "configs" PATH_SEP "safe-conversion.sv");
    AUTOCODE_SM(HARNESS_PATH PATH_SEP "configs" PATH_SEP "safe-conversion.sm");
    RUN_HARNESS("10");
    CHECK_HARNESS_OUT(10);
}

///
/// @test Passing a null state machine assembly to the autocoder returns an
/// error.
///
TEST(StateMachineAutocoder, ErrorNullStateMachineAssembly)
{
    std::stringstream ss;
    CHECK_ERROR(E_SMA_NULL, StateMachineAutocoder::code(ss, "foo", nullptr));
}
