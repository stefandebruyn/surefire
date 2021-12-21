#include "sfa/sv/StateVector.hpp"
#include "sfa/util/MemoryOps.hpp"

#define GET_ELEMENT_IMPL(kTemplateType, kElementType)                          \
    /* Check that the state vector is initialized. */                          \
    if (mConfig.elems == nullptr)                                              \
    {                                                                          \
        return E_UNINITIALIZED;                                                \
    }                                                                          \
                                                                               \
    /* Look up index of specified element. */                                  \
    U32 elemIdx;                                                               \
    const Result res = this->getElementIndex(kName, elemIdx);                  \
    if (res != SUCCESS)                                                        \
    {                                                                          \
        /* Lookup failed. */                                                   \
        return res;                                                            \
    }                                                                          \
                                                                               \
    /* Check that type of element matches template parameter. */               \
    IElement* ielem = mConfig.elems[elemIdx].elem;                             \
    if (ielem == nullptr)                                                      \
    {                                                                          \
        /* This should never happen assuming the state vector config was       \
           validated correctly. */                                             \
        return E_NULLPTR;                                                      \
    }                                                                          \
    if (ielem->getElementType() != kElementType)                               \
    {                                                                          \
        /* Type mismatch. */                                                   \
        return E_TYPE;                                                         \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        /* Type match- narrow reference to element template instantiation. */  \
        kElem = (Element<kTemplateType>*) ielem;                               \
    }                                                                          \
                                                                               \
    return SUCCESS;

Result StateVector::create(const StateVector::Config kConfig, StateVector& kSv)
{
    Result res = E_UNREACHABLE;
    kSv = StateVector(kConfig, res);
    return res;
}

StateVector::StateVector() : mConfig({nullptr, nullptr})
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
    if (mConfig.elems == nullptr)
    {
        return E_UNINITIALIZED;
    }

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

StateVector::StateVector(const Config kConfig, Result& kRes) : StateVector()
{
    kRes = SUCCESS;

    // Check that elements array is non-null.
    if (kConfig.elems == nullptr)
    {
        kRes = E_NULLPTR;
        return;
    }

    // Check that each element config element pointer is non-null.
    U32 i = 0;
    while (kConfig.elems[i].name != nullptr)
    {
        if (kConfig.elems[i].elem == nullptr)
        {
            kRes = E_NULLPTR;
            return;
        }
        ++i;
    }

    if (kConfig.regions != nullptr)
    {
        // Check that each region config region pointer is non-null.
        i = 0;
        while (kConfig.regions[i].name != nullptr)
        {
            if (kConfig.regions[i].region == nullptr)
            {
                kRes = E_NULLPTR;
                return;
            }
            ++i;
        }

        // Check that element memory exactly spans regions memory.
        i = 0;
        U32 elemIdx = 0;
        while (kConfig.regions[i].name != nullptr)
        {
            const U32 regionSize = kConfig.regions[i].region->getSizeBytes();
            const void* regionPtr = kConfig.regions[i].region->getAddr();
            const char* bumpPtr = static_cast<const char*>(regionPtr);

            // Advance through elements array until either the end of the array
            // or the bump pointer passes the end of the current region.
            while ((kConfig.elems[elemIdx].name != nullptr)
                   && (((U64) bumpPtr - (U64) regionPtr) < regionSize))
            {
                // Check that current element address is at the bump pointer.
                if (bumpPtr != kConfig.elems[elemIdx].elem->getAddr())
                {
                    kRes = E_LAYOUT;
                    return;
                }

                // Bump pointer by size of element.
                bumpPtr += kConfig.elems[elemIdx].elem->getSizeBytes();
                ++elemIdx;
            }

            // Check that bump pointer was bumped to exactly the end of the
            // region.
            if (((U64) bumpPtr - (U64) regionPtr) != regionSize)
            {
                kRes = E_LAYOUT;
                return;
            }

            ++i;
        }
    }

    // If we got this far, the config is valid- set `mConfig` so that the state
    // vector is usable.
    mConfig = kConfig;
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
