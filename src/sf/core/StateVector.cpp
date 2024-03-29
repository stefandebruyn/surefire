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
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/MemOps.hpp"
#include "sf/core/StateVector.hpp"

namespace Sf
{

Result StateVector::init(const Config kConfig, StateVector& kSv)
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

    for (U32 i = 0; kConfig.elems[i].name != nullptr; ++i)
    {
        // Check that element pointer is non-null.
        if (kConfig.elems[i].elem == nullptr)
        {
            return E_SV_NULL;
        }

        // Check that element name is unique.
        for (U32 j = (i + 1); kConfig.elems[j].name != nullptr; ++j)
        {
            if (MemOps::strcmp(kConfig.elems[i].name, kConfig.elems[j].name)
                == 0)
            {
                return E_SV_ELEM_DUPE;
            }
        }
    }

    if (kConfig.regions != nullptr)
    {
        for (U32 i = 0; kConfig.regions[i].name != nullptr; ++i)
        {
            // Check that each region config region pointer is non-null.
            if (kConfig.regions[i].region == nullptr)
            {
                return E_SV_NULL;
            }

            // Check that region name is unique.
            for (U32 j = (i + 1); kConfig.regions[j].name != nullptr; ++j)
            {
                if (MemOps::strcmp(kConfig.regions[i].name,
                                   kConfig.regions[j].name) == 0)
                {
                    return E_SV_RGN_DUPE;
                }
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

    // Config is valid- put config in state vector to initialize it.
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

template<>
Result StateVector::getElement<F64>(const char* const kName,
                                    Element<F64>*& kElem)
{
    return this->getElementImpl<F64>(kName, kElem, ElementType::FLOAT64);
}

template<>
Result StateVector::getElement<bool>(const char* const kName,
                                     Element<bool>*& kElem)
{
    return this->getElementImpl<bool>(kName, kElem, ElementType::BOOL);
}

Result StateVector::getIElement(const char* const kName, IElement*& kElem)
{
    // Check that state vector is initialized.
    if (mConfig.elems == nullptr)
    {
        return E_SV_UNINIT;
    }

    // Look up element config.
    const ElementConfig* elemConfig = nullptr;
    const Result res = this->getElementConfig(kName, elemConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    SF_SAFE_ASSERT(elemConfig != nullptr);
    SF_SAFE_ASSERT(elemConfig->elem != nullptr);

    // Return element object.
    kElem = elemConfig->elem;

    return SUCCESS;
}

Result StateVector::getRegion(const char* const kName, Region*& kRegion)
{
    // Check that state vector is initialized.
    if (mConfig.elems == nullptr)
    {
        return E_SV_UNINIT;
    }

    // Check that regions were configured.
    if (mConfig.regions == nullptr)
    {
        return E_SV_EMPTY;
    }

    // Look up region config.
    const RegionConfig* regionConfig = nullptr;
    Result res = this->getRegionConfig(kName, regionConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    SF_SAFE_ASSERT(regionConfig != nullptr);
    SF_SAFE_ASSERT(regionConfig->region != nullptr);

    // Return region object.
    kRegion = regionConfig->region;

    return SUCCESS;
}

Result StateVector::getElementConfig(const char* const kName,
                                     const ElementConfig*& kElemConfig) const
{
    SF_SAFE_ASSERT(mConfig.elems != nullptr);

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
    SF_SAFE_ASSERT(mConfig.regions != nullptr);

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

} // namespace Sf
