#ifndef SFA_STATE_VECTOR_HPP
#define SFA_STATE_VECTOR_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sv/Region.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class StateVector final
{
public:

    struct ElementConfig
    {
        const char* name;
        IElement* elem;
    };

    struct RegionConfig
    {
        const char* name;
        Region* region;
    };

    struct Config
    {
        ElementConfig* elems;
        RegionConfig* regions;
    };

    StateVector(const Config kConfig);

    StateVector(const StateVector&) = delete;
    StateVector& operator=(const StateVector&) = delete;
    StateVector(StateVector&&) = delete;
    StateVector& operator=(StateVector&&) = delete;

    template<typename T>
    Result getElement(const char* const kName, Element<T>*& kELem);

    Result getRegion(const char* const kName, Region*& kRegion);

private:

    Config mConfig;

    Result getElementIndex(const char* const kName, U32& kIdx) const;

    Result getRegionIndex(const char* const kName, U32& kIdx) const;
};

#endif
