#ifndef SFA_STATE_VECTOR_HPP
#define SFA_STATE_VECTOR_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sv/Region.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

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

    static Result create(const Config kConfig, StateVector& kSm);

    StateVector();

    StateVector(const StateVector&) = delete;
    StateVector& operator=(const StateVector&) = delete;
    StateVector(StateVector&&) = delete;

    template<typename T>
    Result getElement(const char* const kName, Element<T>*& kELem);

    Result getRegion(const char* const kName, Region*& kRegion);

private:

    Config mConfig;

    StateVector(const Config kConfig, Result& kRes);

    StateVector& operator=(StateVector&&) = default;

    Result getElementConfig(const char* const kName,
                            ElementConfig*& kElemConfig) const;

    Result getRegionConfig(const char* const kName,
                           RegionConfig*& kRegionConfig) const;
};

#endif
