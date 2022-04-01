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
/// SF_REPO_PATH is set by the CMake project.
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
/// @param[in] kSrc  State vector config as string literal.
///
#define AUTOCODE_SV(kPath)                                                     \
    /* Compile state vector. */                                                \
    String path = kPath;                                                       \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorAssembly::compile(path, svAsm, nullptr));         \
                                                                               \
    /* Randomize state vector element values using the same function as the    \
       harness. */                                                             \
    randomizeStateVector(svAsm->config());                                     \
                                                                               \
    /* Set initial global time, which we don't want randomized, back to 0. */  \
    Element<U64>* elemGlobalTime = nullptr;                                    \
    CHECK_SUCCESS(svAsm->get()->getElement("time", elemGlobalTime));           \
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
/// @param[in] kSrc  State machine config as string literal.
///
#define AUTOCODE_SM(kPath)                                                     \
    /* Compile state machine. */                                               \
    path = kPath;                                                              \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineAssembly::compile(path, svAsm, smAsm, nullptr)); \
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
#define RUN_HARNESS                                                            \
    /* Build and run harness. */                                               \
    const I32 status = std::system(                                            \
        "cd " HARNESS_PATH " && make && ./a.out > " HARNESS_OUT_PATH);         \
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
#define CHECK_HARNESS_OUT                                                      \
    std::stringstream expectOut;                                               \
    /* Fix floating output precision for consistent output. The precision      \
       should match the precision used by the harness. */                      \
    expectOut << std::setprecision(std::numeric_limits<F64>::digits10);        \
    runStateMachine(svAsm, smAsm, expectOut);                                  \
    CHECK_EQUAL(expectOut.str(), hout.str());

///
/// @brief Number of steps which the state machine under test runs for. This
/// should match the equivalent constant in the harness.
///
static const U32 gSmSteps = 1000;

///
/// @brief Prints the name and value of state vector elements in the order
/// configured. This should print a state vector in the exact same way as the
/// harness.
///
/// @param[in]  kSvConfig  Config of state vector to print.
/// @param[out] kOs        Output stream to print to.
///
static void printStateVector(const StateVector::Config kSvConfig,
                             std::ostream& kOs)
{
    for (const StateVector::ElementConfig* elemConfig = kSvConfig.elems;
         elemConfig->name != nullptr;
         ++elemConfig)
    {
        kOs << elemConfig->name << " ";

        const IElement* const elem = elemConfig->elem;
        switch (elem->type())
        {
            case ElementType::INT8:
                kOs << static_cast<I32>(
                    static_cast<const Element<I8>*>(elem)->read()) << "\n";
                break;

            case ElementType::INT16:
                kOs << static_cast<const Element<I16>*>(elem)->read() << "\n";
                break;

            case ElementType::INT32:
                kOs << static_cast<const Element<I32>*>(elem)->read() << "\n";
                break;

            case ElementType::INT64:
                kOs << static_cast<const Element<I64>*>(elem)->read() << "\n";
                break;

            case ElementType::UINT8:
                kOs << static_cast<I32>(
                    static_cast<const Element<U8>*>(elem)->read()) << "\n";
                break;

            case ElementType::UINT16:
                kOs << static_cast<const Element<U16>*>(elem)->read() << "\n";
                break;

            case ElementType::UINT32:
                kOs << static_cast<const Element<U32>*>(elem)->read() << "\n";
                break;

            case ElementType::UINT64:
                kOs << static_cast<const Element<U64>*>(elem)->read() << "\n";
                break;

            case ElementType::FLOAT32:
                kOs << static_cast<const Element<F32>*>(elem)->read() << "\n";
                break;

            case ElementType::FLOAT64:
                kOs << static_cast<const Element<F64>*>(elem)->read() << "\n";
                break;

            case ElementType::BOOL:
                kOs << static_cast<const Element<bool>*>(elem)->read() << "\n";
                break;
        }
    }
}

///
/// @brief Runs a state machine compiled in-memory in the exact same way the
/// harness runs its autocoded state machine. The state vector is printed to
/// a specified output stream each step in the exact same format used by the
/// harness.
///
/// @param[in]  kSvAsm  State vector assembly used by state machine.
/// @param[in]  kSmAsm  State machine assembly to run.
/// @param[out] kOs     Output stream to print state vector to.
///
static void runStateMachine(const Ref<const StateVectorAssembly> kSvAsm,
                            const Ref<const StateMachineAssembly> kSmAsm,
                            std::ostream& kOs)
{
    const Ref<StateVector> sv = kSvAsm->get();
    Element<U64>* elemGlobalTime = nullptr;
    CHECK_SUCCESS(sv->getElement("time", elemGlobalTime));

    const Ref<StateMachine> sm = kSmAsm->get();
    for (U32 i = 0; i < gSmSteps; ++i)
    {
        // Increment global time. Modulate the increment to test state machine
        // behavior with varying delta T.
        const U64 deltaT = ((i % 3) + 1);
        elemGlobalTime->write(elemGlobalTime->read() + deltaT);

        // Step state machine.
        CHECK_SUCCESS(sm->step());

        // Print state vector.
        kOs << "---- STEP " << i << " ----\n";
        printStateVector(kSvAsm->config(), kOs);
    }
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Tests which compile in-memory and autocode state machines from the
/// same config, run them for many steps with randomized inputs, and compare
/// their output. State machines compiled in-memory are known correct from
/// other tests, so they are used as ground truth for autocoded state machine
/// behavior.
///
/// @note Preconditions for all tests:
///
///   1. The global time element is named "time"
///   2. The state element is named "state"
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
/// @test State machine with a bunch of random, complex logic meant to exercise
/// the full range of language syntax. Each state machine step, each local
/// element gets copied into a corresponding state vector element so that local
/// elements are visible to the harness.
///
TEST(StateMachineAutocoder, Nonsense)
{
    AUTOCODE_SV(HARNESS_PATH PATH_SEP "configs" PATH_SEP "nonsense.sv");
    AUTOCODE_SM(HARNESS_PATH PATH_SEP "configs" PATH_SEP "nonsense.sm");
    RUN_HARNESS;
    CHECK_HARNESS_OUT;
}