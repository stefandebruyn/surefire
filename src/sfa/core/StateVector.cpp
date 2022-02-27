#include "sfa/core/StateVector.hpp"
#include "sfa/core/MemOps.hpp"

Result StateVector::create(const Config kConfig, StateVector& kSv)
{
    // Check that state vector is not already initialized.
    if (kSv.mConfig.elems != nullptr)
    {
        return E_SV_REINIT;
    }

    // Check that elements array is non-null.
    if (kConfig.elems == nullptr)
    {
        return E_SV_NULL;
    }

    // Check that each element config element pointer is non-null.
    for (U32 i = 0; kConfig.elems[i].name != nullptr; ++i)
    {
        if (kConfig.elems[i].elem == nullptr)
        {
            return E_SV_NULL;
        }
    }

    if (kConfig.regions != nullptr)
    {
        // Check that each region config region pointer is non-null.
        for (U32 i = 0; kConfig.regions[i].name != nullptr; ++i)
        {
            if (kConfig.regions[i].region == nullptr)
            {
                return E_SV_NULL;
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
                    return E_SV_LAYOUT;
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
                return E_SV_LAYOUT;
            }
        }
    }

    // If we got this far, the config is valid- set `mConfig` so that the state
    // vector is usable.
    kSv.mConfig = kConfig;
    return SUCCESS;
}

StateVector::StateVector() : mConfig({nullptr, nullptr})
{
}

template<>
Result StateVector::getElement<I8>(const char* const kName, Element<I8>*& kElem)
{
    return this->getElementImpl<I8>(kName, kElem, ElementType::INT8);
}

template<>
Result StateVector::getElement<I16>(const char* const kName,
                                    Element<I16>*& kElem)
{
    return this->getElementImpl<I16>(kName, kElem, ElementType::INT16);
}

template<>
Result StateVector::getElement<I32>(const char* const kName,
                                    Element<I32>*& kElem)
{
    return this->getElementImpl<I32>(kName, kElem, ElementType::INT32);
}

template<>
Result StateVector::getElement<I64>(const char* const kName,
                                    Element<I64>*& kElem)
{
    return this->getElementImpl<I64>(kName, kElem, ElementType::INT64);
}

template<>
Result StateVector::getElement<U8>(const char* const kName, Element<U8>*& kElem)
{
    return this->getElementImpl<U8>(kName, kElem, ElementType::UINT8);
}

template<>
Result StateVector::getElement<U16>(const char* const kName,
                                    Element<U16>*& kElem)
{
    return this->getElementImpl<U16>(kName, kElem, ElementType::UINT16);
}

template<>
Result StateVector::getElement<U32>(const char* const kName,
                                    Element<U32>*& kElem)
{
    return this->getElementImpl<U32>(kName, kElem, ElementType::UINT32);
}

template<>
Result StateVector::getElement<U64>(const char* const kName,
                                    Element<U64>*& kElem)
{
    return this->getElementImpl<U64>(kName, kElem, ElementType::UINT64);
}

template<>
Result StateVector::getElement<F32>(const char* const kName,
                                    Element<F32>*& kElem)
{
    return this->getElementImpl<F32>(kName, kElem, ElementType::FLOAT32);
}

#ifndef SFA_NO_F64
template<>
Result StateVector::getElement<F64>(const char* const kName,
                                    Element<F64>*& kElem)
{
    return this->getElementImpl<F64>(kName, kElem, ElementType::FLOAT64);
}
#endif

template<>
Result StateVector::getElement<bool>(const char* const kName,
                                     Element<bool>*& kElem)
{
    return this->getElementImpl<bool>(kName, kElem, ElementType::BOOL);
}

Result StateVector::getIElement(const char* const kName,
                                const IElement*& kElem) const
{
    const ElementConfig* elemConfig = nullptr;
    const Result res = this->getElementConfig(kName, elemConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    kElem = elemConfig->elem;
    return SUCCESS;
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

    const RegionConfig* regionConfig = nullptr;
    Result res = this->getRegionConfig(kName, regionConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    kRegion = regionConfig->region;
    return SUCCESS;
}

Result StateVector::getElementConfig(const char* const kName,
                                     const ElementConfig*& kElemConfig) const
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
                                    const RegionConfig*& kRegionConfig) const
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
