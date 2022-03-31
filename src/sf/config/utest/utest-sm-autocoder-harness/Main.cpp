#include "FooStateMachine.hpp"
#include "FooStateVector.hpp"
#include "sf/pal/Console.hpp"

static const U32 gSmSteps = 1000;

static void printStateVector(const StateVector::Config kSvConfig)
{
    for (const StateVector::ElementConfig* elemConfig = kSvConfig.elems;
         elemConfig->name != nullptr;
         ++elemConfig)
    {
        Console::printf("%s ", elemConfig->name);

        const IElement* const elem = elemConfig->elem;
        switch (elem->type())
        {
            case ElementType::INT8:
                Console::printf("%d\n",
                    static_cast<const Element<I8>*>(elem)->read());
                break;

            case ElementType::INT16:
                Console::printf("%d\n",
                    static_cast<const Element<I16>*>(elem)->read());
                break;

            case ElementType::INT32:
                Console::printf("%d\n",
                    static_cast<const Element<I32>*>(elem)->read());
                break;

            case ElementType::INT64:
                Console::printf("%ll\n",
                    static_cast<const Element<I64>*>(elem)->read());
                break;

            case ElementType::UINT8:
                Console::printf("%d\n",
                    static_cast<const Element<U8>*>(elem)->read());
                break;

            case ElementType::UINT16:
                Console::printf("%d\n",
                    static_cast<const Element<U16>*>(elem)->read());
                break;

            case ElementType::UINT32:
                Console::printf("%u\n",
                    static_cast<const Element<U32>*>(elem)->read());
                break;

            case ElementType::UINT64:
                Console::printf("%lld\n",
                    static_cast<const Element<U64>*>(elem)->read());
                break;

            case ElementType::FLOAT32:
                Console::printf("%f\n",
                    static_cast<const Element<F32>*>(elem)->read());
                break;

            case ElementType::FLOAT64:
                Console::printf("%f\n",
                    static_cast<const Element<F64>*>(elem)->read());
                break;

            case ElementType::BOOL:
                Console::printf("%d\n",
                    static_cast<const Element<bool>*>(elem)->read());
                break;
        }
    }
}

I32 main(I32 kArgc, char* kArgv[])
{
    // Get autocoded state vector config.
    StateVector::Config svConfig;
    Result res = FooStateVector::getConfig(svConfig);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }

    // Initialize state vector.
    StateVector sv;
    res = StateVector::create(svConfig, sv);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }

    // Set initial state.
    Element<U32>* elemState = nullptr;
    res = sv.getElement("state", elemState);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }
    elemState->write(1);

    // Get global time element.
    Element<U64>* elemGlobalTime = nullptr;
    res = sv.getElement("time", elemGlobalTime);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }

    // Get autocoded state machine config.
    StateMachine::Config smConfig;
    res = FooStateMachine::getConfig(sv, smConfig);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }

    // Initialize state machine.
    StateMachine sm;
    res = StateMachine::create(smConfig, sm);
    if (res != SUCCESS)
    {
        Console::printf("error %d\n", res);
        return 1;
    }

    // Run state machine for the configured number of steps and print state
    // vector at each step.
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
            Console::printf("error %d\n", res);
            return 1;
        }

        // Print state vector.
        Console::printf("---- STEP %u ----\n", i);
        printStateVector(svConfig);
    }

    return 0;
}
