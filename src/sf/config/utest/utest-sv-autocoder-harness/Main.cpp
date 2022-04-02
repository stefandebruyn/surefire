///
/// @brief Harness for testing an autocoded state vector configuration. Autocode
/// is in the same directory in a file named `FooStateVector.hpp`. Expected
/// state vector element names are passed on command line, followed by expected
/// region names prefixed with `.`s. The harness prints the type and name of
/// each element, and name and size of each region on separate lines. The
/// harness also does a basic read/write/read on each element. On error, the
/// harness exits with an error code.
///

#include <cstdlib>
#include <iostream>

#include "FooStateVector.hpp"

///
/// @brief Exits with an error status if two values are not equal.
///
/// @param[in] kExpect  Expected value.
/// @param[in] kActual  Actual value.
///
#define CHECK_ELEM(kExpect, kActual)                                           \
if (kExpect != kActual)                                                        \
{                                                                              \
    std::cout << kArgv[i] << " had unexpected value " << kActual << "\n";      \
    return 1;                                                                  \
}

///
/// @brief Element type name lookup table, indexed by `ElementType`.
///
static const char* const gElemTypeNames[] =
{
    "(none)",
    "I8",
    "I16",
    "I32",
    "I64",
    "U8",
    "U16",
    "U32",
    "U64",
    "F32",
    "F64",
    "bool"
};

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

    // Initialize state vector.
    StateVector sv;
    res = StateVector::init(svConfig, sv);
    if (res != SUCCESS)
    {
        std::cout << "error " << res << "\n";
        return 1;
    }

    // Loop through args, verifying elements and regions.
    IElement* elemObj = nullptr;
    for (I32 i = 1; i < kArgc; ++i)
    {
        if (kArgv[i][0] == '.')
        {
            // Arg is a region.
            Region* region = nullptr;
            res = sv.getRegion((kArgv[i] + 1), region);
            if (res != SUCCESS)
            {
                std::cout << "error " << res << "\n";
                return 1;
            }

            // Print region name and size.
            std::cout << (kArgv[i] + 1) << " " << region->size() << "\n";

            continue;
        }

        // Arg is an element.
        res = sv.getIElement(kArgv[i], elemObj);
        if (res != SUCCESS)
        {
            std::cout << "error " << res << "\n";
            return 1;
        }

        // Print element type and name.
        std::cout << gElemTypeNames[elemObj->type()] << " " << kArgv[i]
                  << "\n";

        // Check that each element defaulted to zero and can be written and read
        // again.
        switch (elemObj->type())
        {
            case ElementType::INT8:
            {
                Element<I8>& elem = *static_cast<Element<I8>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::INT16:
            {
                Element<I16>& elem = *static_cast<Element<I16>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::INT32:
            {
                Element<I32>& elem = *static_cast<Element<I32>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::INT64:
            {
                Element<I64>& elem = *static_cast<Element<I64>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::UINT8:
            {
                Element<U8>& elem = *static_cast<Element<U8>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::UINT16:
            {
                Element<U16>& elem = *static_cast<Element<U16>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::UINT32:
            {
                Element<U32>& elem = *static_cast<Element<U32>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::UINT64:
            {
                Element<U64>& elem = *static_cast<Element<U64>*>(elemObj);
                CHECK_ELEM(0, elem.read());
                elem.write(100);
                CHECK_ELEM(100, elem.read());
                break;
            }

            case ElementType::FLOAT32:
            {
                Element<F32>& elem = *static_cast<Element<F32>*>(elemObj);
                CHECK_ELEM(0.0f, elem.read());
                elem.write(100.0f);
                CHECK_ELEM(100.0f, elem.read());
                break;
            }

            case ElementType::FLOAT64:
            {
                Element<F64>& elem = *static_cast<Element<F64>*>(elemObj);
                CHECK_ELEM(0.0, elem.read());
                elem.write(100.0);
                CHECK_ELEM(100.0, elem.read());
                break;
            }

            case ElementType::BOOL:
            {
                Element<bool>& elem = *static_cast<Element<bool>*>(elemObj);
                CHECK_ELEM(false, elem.read());
                elem.write(true);
                CHECK_ELEM(true, elem.read());
                break;
            }

            default:
                // Unreachable.
                return 1;
        }
    }

    return 0;
}
