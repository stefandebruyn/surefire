#include "sfa/statevec/StateVector.hpp"
#include "sfa/util/MemoryOps.hpp"

#define GET_ELEMENT_IMPL(kTemplateType, kElementType)                          \
    /* Look up index of specified element. */                                  \
    U32 elemIdx;                                                               \
    Result res = this->getElementIndex(kName, elemIdx);                        \
    if (res != SUCCESS)                                                        \
    {                                                                          \
        /* Lookup failed. */                                                   \
        return res;                                                            \
    }                                                                          \
                                                                               \
    /* Check that type of element matches template parameter. */               \
    IElement& ielem = *(mConfig.elems[elemIdx].elem);                          \
    res = SUCCESS;                                                             \
    if (ielem.getElementType() != kElementType)                                \
    {                                                                          \
        /* Type mismatch. */                                                   \
        res = E_TYPE;                                                          \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        /* Type match- narrow reference to element template instantiation. */  \
        kElem = (Element<kTemplateType>*) &ielem;                              \
    }                                                                          \
                                                                               \
    return res;

StateVector::StateVector(const Config kConfig) : mConfig(kConfig)
{
}

template<>
Result StateVector::getElement<I8>(const char* const kName, Element<I8>*& kElem)
{
    GET_ELEMENT_IMPL(I8, ElementType::INT8);
}

template<>
Result StateVector::getElement<I16>(const char* const kName,
                                    Element<I16>*& kElem)
{
    GET_ELEMENT_IMPL(I16, ElementType::INT16);
}

template<>
Result StateVector::getElement<I32>(
    const char* const kName, Element<I32>*& kElem)
{
    GET_ELEMENT_IMPL(I32, ElementType::INT32);
}

template<>
Result StateVector::getElement<I64>(const char* const kName,
                                    Element<I64>*& kElem)
{
    GET_ELEMENT_IMPL(I64, ElementType::INT64);
}

template<>
Result StateVector::getElement<U8>(const char* const kName, Element<U8>*& kElem)
{
    GET_ELEMENT_IMPL(U8, ElementType::UINT8);
}

template<>
Result StateVector::getElement<U16>(const char* const kName,
                                    Element<U16>*& kElem)
{
    GET_ELEMENT_IMPL(U16, ElementType::UINT16);
}

template<>
Result StateVector::getElement<U32>(const char* const kName,
                                    Element<U32>*& kElem)
{
    GET_ELEMENT_IMPL(U32, ElementType::UINT32);
}

template<>
Result StateVector::getElement<U64>(const char* const kName,
                                    Element<U64>*& kElem)
{
    GET_ELEMENT_IMPL(U64, ElementType::UINT64);
}

template<>
Result StateVector::getElement<F32>(const char* const kName,
                                    Element<F32>*& kElem)
{
    GET_ELEMENT_IMPL(F32, ElementType::FLOAT32);
}

template<>
Result StateVector::getElement<F64>(const char* const kName,
                                    Element<F64>*& kElem)
{
    GET_ELEMENT_IMPL(F64, ElementType::FLOAT64);
}

template<>
Result StateVector::getElement<bool>(const char* const kName,
                                     Element<bool>*& kElem)
{
    GET_ELEMENT_IMPL(bool, ElementType::BOOL);
}

Result StateVector::getRegion(const char* const kName, Region*& kRegion)
{
    if (mConfig.regions == nullptr)
    {
        return E_EMPTY;
    }

    U32 idx;
    Result res = this->getRegionIndex(kName, idx);
    if (res != SUCCESS)
    {
        return res;
    }

    kRegion = mConfig.regions[idx].region;
    return SUCCESS;
}

Result StateVector::getElementIndex(const char* const kName, U32& kIdx) const
{
    // Look up index of element config by name.
    U32 i = 0;
    while (mConfig.elems[i].name != nullptr)
    {
        if (Sfa::stringsEqual(mConfig.elems[i].name, kName) == true)
        {
            kIdx = i;
            return SUCCESS;
        }

        ++i;
    }

    // If we get this far, the element wasn't found.
    return E_KEY;
}

Result StateVector::getRegionIndex(const char* const kName, U32& kIdx) const
{
    // Look up index of region config by name.
    U32 i = 0;
    while (mConfig.regions[i].name != nullptr)
    {
        if (Sfa::stringsEqual(mConfig.regions[i].name, kName) == true)
        {
            kIdx = i;
            return SUCCESS;
        }

        ++i;
    }

    // If we get this far, the region wasn't found.
    return E_KEY;
}
