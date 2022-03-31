#include <fstream>
#include <sstream>

#include "sf/config/StateMachineAutocoder.hpp"
#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

#define HARNESS_PATH                                                           \
    SF_REPO_PATH PATH_SEP "src" PATH_SEP "sf" PATH_SEP "config" PATH_SEP       \
    "utest" PATH_SEP "utest-sm-autocoder-harness"

#define HARNESS_OUT_PATH HARNESS_PATH PATH_SEP "out.tmp"

#define HARNESS_BIN_PATH HARNESS_PATH PATH_SEP "a.out"

#define SV_AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateVector.hpp"

#define SM_AUTOCODE_PATH HARNESS_PATH PATH_SEP "FooStateMachine.hpp"

#define AUTOCODE_SV(kSrc)                                                      \
    /* Parse state vector config. */                                           \
    TOKENIZE(kSrc);                                                            \
    Ref<const StateVectorParse> svParse;                                       \
    CHECK_SUCCESS(StateVectorParse::parse(toks, svParse, nullptr));            \
                                                                               \
    /* Generate state vector autocode. */                                      \
    std::ofstream ofs(SV_AUTOCODE_PATH, std::fstream::out);                    \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(                                                             \
        StateVectorAutocoder::code(ofs, "FooStateVector", svParse, nullptr));  \
    ofs.close();

#define AUTOCODE_SM(kSrc)                                                      \
    /* Compile state vector. */                                                \
    Ref<const StateVectorAssembly> svAsm;                                      \
    CHECK_SUCCESS(StateVectorAssembly::compile(svParse, svAsm, nullptr));      \
                                                                               \
    /* Compile state machine. */                                               \
    std::stringstream ss(kSrc);                                                \
    Ref<const StateMachineAssembly> smAsm;                                     \
    CHECK_SUCCESS(StateMachineAssembly::compile(ss, svAsm, smAsm, nullptr));   \
                                                                               \
    /* Generate state machine autocode. */                                     \
    ofs.open(SM_AUTOCODE_PATH, std::fstream::out);                             \
    CHECK_TRUE(ofs.is_open());                                                 \
    CHECK_SUCCESS(                                                             \
        StateMachineAutocoder::code(ofs, "FooStateMachine", smAsm));           \
    ofs.close();

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

#define CHECK_HARNESS_OUT                                                      \
    std::stringstream expectOut;                                               \
    runStateMachine(svAsm, smAsm, expectOut);                                  \
    CHECK_EQUAL(expectOut.str(), hout.str());

static const U32 gSmSteps = 1000;

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
                kOs << static_cast<const Element<I8>*>(elem)->read() << "\n";
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
                kOs << static_cast<const Element<U8>*>(elem)->read() << "\n";
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

TEST(StateMachineAutocoder, Doop)
{
    AUTOCODE_SV(
        "[Foo]\n"
        "U64 time\n"
        "U32 state\n"
        "BOOL qux\n");
    AUTOCODE_SM(
        "[STATE_VECTOR]\n"
        "U64 time @ALIAS G\n"
        "U32 state @ALIAS S\n"
        "BOOL qux @ALIAS corge"
        "\n"
        "[LOCAL]\n"
        "F64 fib = 1.522\n"
        "I32 foo = 0\n"
        "F64 bar = 100 * 100 + fib\n"
        "BOOL baz = 0\n"
        "U32 windowSize = 4\n"
        "\n"
        "[Foo]\n"
        ".ENTRY\n"
        "    bar + 100 < ROLL_AVG(foo, windowSize * 2) OR baz: foo = 1\n"
        "    corge = TRUE\n"
        ".STEP\n"
        "    G == 10: -> Bar\n"
        "\n"
        "[Bar]\n");
    RUN_HARNESS;
    CHECK_HARNESS_OUT;
}
