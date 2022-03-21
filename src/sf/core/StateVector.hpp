#ifndef SF_STATE_VECTOR_HPP
#define SF_STATE_VECTOR_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Region.hpp"
#include "sf/core/Result.hpp"

class StateVector final
{
public:

    struct ElementConfig final
    {
        const char* name;
        IElement* elem;
    };

    struct RegionConfig final
    {
        const char* name;
        Region* region;
    };

    struct Config final
    {
        ElementConfig* elems;
        RegionConfig* regions;
    };

    static Result create(const Config kConfig, StateVector& kSv);

    StateVector();

    template<typename T>
    Result getElement(const char* const kName, Element<T>*& kELem);

    Result getIElement(const char* const kName, IElement*& kElem) const;

    Result getRegion(const char* const kName, Region*& kRegion);

    StateVector(const StateVector&) = delete;
    StateVector(StateVector&&) = delete;
    StateVector& operator=(const StateVector&) = delete;
    StateVector& operator=(StateVector&&) = default;

private:

    Config mConfig;

    Result getElementConfig(const char* const kName,
                            const ElementConfig*& kElemConfig) const;

    Result getRegionConfig(const char* const kName,
                           const RegionConfig*& kRegionConfig) const;

    template<typename T>
    Result getElementImpl(const char* const kName,
                          Element<T>*& kElem,
                          const ElementType kElemType)
    {
        if (mConfig.elems == nullptr)
        {
            return E_SV_UNINIT;
        }

        const ElementConfig* elemConfig = nullptr;
        const Result res = this->getElementConfig(kName, elemConfig);
        if (res != SUCCESS)
        {
            return res;
        }

        IElement* const elem = elemConfig->elem;
        if (elem == nullptr)
        {
            return E_SV_NULL;
        }

        if (elem->type() != kElemType)
        {
            return E_SV_TYPE;
        }

        kElem = static_cast<Element<T>*>(elem);
        return SUCCESS;
    }
};

#endif
