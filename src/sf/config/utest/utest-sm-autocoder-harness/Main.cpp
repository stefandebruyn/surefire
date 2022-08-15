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
/// @file  sf/config/utest/utest-sm-autocoder-harness/Main.hpp
/// @brief Harness for testing an autocoded state machine configuration.
///        State vector and state machine autocode is in the same directory in
///        files `FooStateVector.hpp` and `FooStateMachine.hpp`. The harness
///        sets the initial state to 1 and runs the state machine with delta T
///        in a repeating pattern 1, 2, 3. Each step the harness prints the
///        state vector (but not the state machine local state vector, which the
///        harness cannot access). The first command line argument specifies the
///        number of steps to run the state machine for, and additional
///        arguments specify the initial values of state vector elements in the
///        form "<elem name>=<initial value>". Prior to setting the specified
///        initial values, state vector data is randomized except for the global
///        time, which is initially 0.
////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>

#include "Common.hpp"
#include "FooStateMachine.hpp"
#include "FooStateVector.hpp"

///
/// @brief Sets the value of a state vector element.
///
/// @param[in] kSv        State vector containing element.
/// @param[in] kElemName  Element name.
/// @param[in] kVal       Set value.
///
static void setElementValue(StateVector& kSv,
                            const char* const kElemName,
                            const F64 kVal)
{
    IElement* elem = nullptr;
    const Result res = kSv.getIElement(kElemName, elem);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        std::exit(1);
    }

    switch (elem->type())
    {
        case ElementType::INT8:
            static_cast<Element<I8>*>(elem)->write(kVal);
            break;

        case ElementType::INT16:
            static_cast<Element<I16>*>(elem)->write(kVal);
            break;

        case ElementType::INT32:
            static_cast<Element<I32>*>(elem)->write(kVal);
            break;

        case ElementType::INT64:
            static_cast<Element<I64>*>(elem)->write(kVal);
            break;

        case ElementType::UINT8:
            static_cast<Element<U8>*>(elem)->write(kVal);
            break;

        case ElementType::UINT16:
            static_cast<Element<U16>*>(elem)->write(kVal);
            break;

        case ElementType::UINT32:
            static_cast<Element<U32>*>(elem)->write(kVal);
            break;

        case ElementType::UINT64:
            static_cast<Element<U64>*>(elem)->write(kVal);
            break;

        case ElementType::FLOAT32:
            static_cast<Element<F32>*>(elem)->write(kVal);
            break;

        case ElementType::FLOAT64:
            static_cast<Element<F64>*>(elem)->write(kVal);
            break;

        case ElementType::BOOL:
            static_cast<Element<bool>*>(elem)->write(kVal);
            break;
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
    resetRandomGenerator();
    randomizeStateVector(svConfig);

    // Initialize state vector.
    StateVector sv;
    res = StateVector::init(svConfig, sv);
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

    // Parse number of steps to run state machine from command line.
    if (kArgc < 2)
    {
        std::cout << "expected at least 1 command line arg\n";
        return 1;
    }
    const U64 smSteps = std::strtoll(kArgv[1], nullptr, 10);

    // Set initial values specified over command line.
    for (I32 i = 2; i < kArgc; ++i)
    {
        const char* const elemName = std::strtok(kArgv[i], "=");
        const char* const elemVal = std::strtok(nullptr, "=");
        setElementValue(sv, elemName, std::strtod(elemVal, nullptr));
    }

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
    res = StateMachine::init(smConfig, sm);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Fix the floating output precision.
    std::cout << std::setprecision(std::numeric_limits<F64>::digits10);

    // Run state machine for the configured number of steps.
    for (U32 i = 0; i < smSteps; ++i)
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
        printStateVector(svConfig, std::cout);
    }

    return 0;
}
