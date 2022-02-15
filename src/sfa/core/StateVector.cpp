#include "sfa/core/StateVector.hpp"
#include "sfa/core/MemOps.hpp"

#define GET_ELEMENT_IMPL(kTemplateType, kElementType)                          \
    /* Check that the state vector is initialized. */                          \
    if (mConfig.elems == nullptr)                                              \
    {                                                                          \
        return E_SV_UNINIT;                                                    \
    }                                                                          \
                                                                               \
    /* Look up index of specified element. */                                  \
    ElementConfig* elemConfig = nullptr;                                       \
    const Result res = this->getElementConfig(kName, elemConfig);              \
    if (res != SUCCESS)                                                        \
    {                                                                          \
        /* Lookup failed. */                                                   \
        return res;                                                            \
    }                                                                          \
                                                                               \
    /* Check that type of element matches template parameter. */               \
    IElement* elem = elemConfig->elem;                                         \
    if (elem == nullptr)                                                       \
    {                                                                          \
        /* This should never happen assuming the state vector config was       \
           validated correctly. */                                             \
        return E_SV_NULL;                                                      \
    }                                                                          \
    if (elem->type() != kElementType)                                          \
    {                                                                          \
        /* Type mismatch. */                                                   \
        return E_SV_TYPE;                                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        /* Type match- narrow reference to element template instantiation. */  \
        kElem = static_cast<Element<kTemplateType>*>(elem);                    \
    }                                                                          \
                                                                               \
    return SUCCESS;

Result StateVector::create(const Config kConfig, StateVector& kSv)
{
    Result res = -1;
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
Result StateVector::getElement<I32>(const char* const kName,
                                    Element<I32>*& kElem)
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

#ifndef SFA_NO_F64
template<>
Result StateVector::getElement<F64>(const char* const kName,
                                    Element<F64>*& kElem)
{
    GET_ELEMENT_IMPL(F64, ElementType::FLOAT64);
}
#endif

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
        return E_SV_UNINIT;
    }

    if (mConfig.regions == nullptr)
    {
        return E_SV_EMPTY;
    }

    RegionConfig* regionConfig = nullptr;
    Result res = this->getRegionConfig(kName, regionConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    kRegion = regionConfig->region;
    return SUCCESS;
}

StateVector::StateVector(const Config kConfig, Result& kRes) : StateVector()
{
    kRes = SUCCESS;

    // Check that elements array is non-null.
    if (kConfig.elems == nullptr)
    {
        kRes = E_SV_NULL;
        return;
    }

    // Check that each element config element pointer is non-null.
    for (U32 i = 0; kConfig.elems[i].name != nullptr; ++i)
    {
        if (kConfig.elems[i].elem == nullptr)
        {
            kRes = E_SV_NULL;
            return;
        }
    }

    if (kConfig.regions != nullptr)
    {
        // Check that each region config region pointer is non-null.
        for (U32 i = 0; kConfig.regions[i].name != nullptr; ++i)
        {
            if (kConfig.regions[i].region == nullptr)
            {
                kRes = E_SV_NULL;
                return;
            }
        }

        // Check that element memory exactly spans regions memory.
        U32 elemIdx = 0;
        for (U32 i = 0; kConfig.regions[i].name != nullptr; ++i)
        {
            const U32 regionSize = kConfig.regions[i].region->size();
            const void* regionPtr = kConfig.regions[i].region->addr();
            const char* bumpPtr = static_cast<const char*>(regionPtr);

            // Advance through elements array until either the end of the array
            // or the bump pointer passes the end of the current region.
            while ((kConfig.elems[elemIdx].name != nullptr)
                   && ((reinterpret_cast<U64>(bumpPtr)
                        - reinterpret_cast<U64>(regionPtr)) < regionSize))
            {
                // Check that current element address is at the bump pointer.
                if (bumpPtr != kConfig.elems[elemIdx].elem->addr())
                {
                    kRes = E_SV_LAYOUT;
                    return;
                }

                // Bump pointer by size of element.
                bumpPtr += kConfig.elems[elemIdx].elem->size();
                ++elemIdx;
            }

            // Check that bump pointer was bumped to exactly the end of the
            // region.
            if ((reinterpret_cast<U64>(bumpPtr)
                 - reinterpret_cast<U64>(regionPtr)) != regionSize)
            {
                kRes = E_SV_LAYOUT;
                return;
            }
        }
    }

    // If we got this far, the config is valid- set `mConfig` so that the state
    // vector is usable.
    mConfig = kConfig;
}

Result StateVector::getElementConfig(const char* const kName,
                                     ElementConfig*& kElemConfig) const
{
    // Look up element config by name.
    for (U32 i = 0; mConfig.elems[i].name != nullptr; ++i)
    {
        if (MemOps::strcmp(mConfig.elems[i].name, kName) == 0)
        {
            kElemConfig = &mConfig.elems[i];
            return SUCCESS;
        }
    }

    // If we get this far, the element wasn't found.
    return E_SV_KEY;
}

Result StateVector::getRegionConfig(const char* const kName,
                                    RegionConfig*& kRegionConfig) const
{
    // Look up region config by name.
    for (U32 i = 0; mConfig.regions[i].name != nullptr; ++i)
    {
        if (MemOps::strcmp(mConfig.regions[i].name, kName) == 0)
        {
            kRegionConfig = &mConfig.regions[i];
            return SUCCESS;
        }
    }

    // If we get this far, the region wasn't found.
    return E_SV_KEY;
}
