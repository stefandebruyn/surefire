///
/// @brief Code shared between the state machine autocoder test harness and the
/// unit test suite.
///

#ifndef UTEST_SM_AUTOCODER_HARNESS_COMMON_HPP
#define UTEST_SM_AUTOCODER_HARNESS_COMMON_HPP

#include <random>
#include <limits>

#include "sf/core/StateVector.hpp"

///
/// @brief Random engine with a constant seed.
///
static std::mt19937 gRandGen(0);

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

#endif