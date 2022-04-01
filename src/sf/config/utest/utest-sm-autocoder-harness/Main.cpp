///
/// @brief Harness for testing an autocoded state machine configuration.
/// State vector and state machine autocode is in the same directory in files
/// `FooStateVector.hpp` and `FooStateMachine.hpp`. The harness sets the initial
/// state to 1 and runs the state machine for gSmSteps steps with delta T in
/// a repeating pattern 1, 2, 3. Each step the harness prints the state vector
/// (but not the state machine local state vector, which the harness cannot
/// access).
///

#include <iomanip>
#include <iostream>
#include <limits>

#include "Common.hpp"
#include "FooStateMachine.hpp"
#include "FooStateVector.hpp"

///
/// @brief Number of steps state machine runs for.
///
static const U32 gSmSteps = 1000;

///
/// @brief Prints the value of state vector elements in the order configured.
///
/// @param[in] kSvConfig  State vector config.
///
static void printStateVector(const StateVector::Config kSvConfig)
{
    for (const StateVector::ElementConfig* elemConfig = kSvConfig.elems;
         elemConfig->name != nullptr;
         ++elemConfig)
    {
        std::cout << elemConfig->name << " ";

        const IElement* const elem = elemConfig->elem;
        switch (elem->type())
        {
            case ElementType::INT8:
                std::cout << static_cast<I32>(
                    static_cast<const Element<I8>*>(elem)->read()) << "\n";
                break;

            case ElementType::INT16:
                std::cout << static_cast<const Element<I16>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::INT32:
                std::cout << static_cast<const Element<I32>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::INT64:
                std::cout << static_cast<const Element<I64>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::UINT8:
                std::cout << static_cast<I32>(
                    static_cast<const Element<U8>*>(elem)->read()) << "\n";
                break;

            case ElementType::UINT16:
                std::cout << static_cast<const Element<U16>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::UINT32:
                std::cout << static_cast<const Element<U32>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::UINT64:
                std::cout << static_cast<const Element<U64>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::FLOAT32:
                std::cout << static_cast<const Element<F32>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::FLOAT64:
                std::cout << static_cast<const Element<F64>*>(elem)->read()
                          << "\n";
                break;

            case ElementType::BOOL:
                std::cout << static_cast<const Element<bool>*>(elem)->read()
                          << "\n";
                break;
        }
    }
}

///
/// @brief Entry point.
///
I32 main(I32 kArgc, char* kArgv[])
{
    // Get autocoded state vector config.
    StateVector::Config svConfig;
    Result res = FooStateVector::getConfig(svConfig);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Randomize state vector element values.
    randomizeStateVector(svConfig);

    // Initialize state vector.
    StateVector sv;
    res = StateVector::create(svConfig, sv);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Set initial state.
    Element<U32>* elemState = nullptr;
    res = sv.getElement("state", elemState);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }
    elemState->write(1);

    // Get global time element.
    Element<U64>* elemGlobalTime = nullptr;
    res = sv.getElement("time", elemGlobalTime);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Set initial global time, which we don't want randomized, back to 0.
    elemGlobalTime->write(0);

    // Get autocoded state machine config.
    StateMachine::Config smConfig;
    res = FooStateMachine::getConfig(sv, smConfig);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Initialize state machine.
    StateMachine sm;
    res = StateMachine::create(smConfig, sm);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Fix the floating output precision.
    std::cout << std::setprecision(std::numeric_limits<F64>::digits10);

    // Run state machine for the configured number of steps.
    for (U32 i = 0; i < gSmSteps; ++i)
    {
        // Increment global time. Modulate the increment to test state machine
        // behavior with varying delta T.
        const U64 deltaT = ((i % 3) + 1);
        elemGlobalTime->write(elemGlobalTime->read() + deltaT);

        // Step state machine.
        res = sm.step();
        if (res != SUCCESS)
        {
            std::cout << "error " << res << "\n";
            return 1;
        }

        // Print state vector.
        std::cout << "---- STEP " << i << " ----\n";
        printStateVector(svConfig);
    }

    return 0;
}
