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
/// @file  sf/config/utest/utest-sm-autocoder-harness/Common.hpp
/// @brief Code shared between the state machine autocoder test harness and the
///        unit test suite.
////////////////////////////////////////////////////////////////////////////////

#ifndef UTEST_SM_AUTOCODER_HARNESS_COMMON_HPP
#define UTEST_SM_AUTOCODER_HARNESS_COMMON_HPP

#include <random>
#include <limits>

#include "sf/core/StateVector.hpp"

///
/// @brief Random generator used by random*() functions.
///
static std::mt19937 gRandGen;

///
/// @brief Resets the random generator with a constant seed.
///
static void resetRandomGenerator()
{
    gRandGen = std::mt19937(0);
}

///
/// @brief Generates a random integer in the representable range of the
/// specified type.
///
/// @tparam T  Integer type.
///
/// @return Random value.
///
template<typename T>
static T randomInt()
{
    std::uniform_int_distribution<T> distr(
        std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
    return distr(gRandGen);
}

///
/// @brief Generates a random float in the representable range of the specified
/// type.
///
/// @tparam T  Floating type.
///
/// @return Random value.
///
template<typename T>
static T randomFloat()
{
    std::uniform_real_distribution<T> distr(-1000000000.0, 1000000000.0);
    return distr(gRandGen);
}

///
/// @brief Generates a random bool.
///
/// @return Random bool.
///
static bool randomBool()
{
    return ((randomInt<U8>() % 2) == 0);
}

///
/// @brief Randomizes all the element values in a state vector.
///
/// @param[in] kSvConfig  Config of state vector to randomize.
///
static void randomizeStateVector(const StateVector::Config& kSvConfig)
{
    for (const StateVector::ElementConfig* elemConfig = kSvConfig.elems;
         elemConfig->name != nullptr;
         ++elemConfig)
    {
        IElement* const elem = elemConfig->elem;
        switch (elem->type())
        {
            case ElementType::INT8:
                static_cast<Element<I8>*>(elem)->write(randomInt<I8>());
                break;

            case ElementType::INT16:
                static_cast<Element<I16>*>(elem)->write(randomInt<I16>());
                break;

            case ElementType::INT32:
                static_cast<Element<I32>*>(elem)->write(randomInt<I32>());
                break;

            case ElementType::INT64:
                static_cast<Element<I64>*>(elem)->write(randomInt<I64>());
                break;

            case ElementType::UINT8:
                static_cast<Element<U8>*>(elem)->write(randomInt<U8>());
                break;

            case ElementType::UINT16:
                static_cast<Element<U16>*>(elem)->write(randomInt<U16>());
                break;

            case ElementType::UINT32:
                static_cast<Element<U32>*>(elem)->write(randomInt<U32>());
                break;

            case ElementType::UINT64:
                static_cast<Element<U64>*>(elem)->write(randomInt<U64>());
                break;

            case ElementType::FLOAT32:
                static_cast<Element<F32>*>(elem)->write(randomFloat<F32>());
                break;

            case ElementType::FLOAT64:
                static_cast<Element<F64>*>(elem)->write(randomFloat<F64>());
                break;

            case ElementType::BOOL:
                static_cast<Element<bool>*>(elem)->write(randomBool());
        }
    }
}

///
/// @brief Prints the name and value of state vector elements in the order
/// configured.
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

#endif
