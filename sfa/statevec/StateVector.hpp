#ifndef SFA_STATE_VECTOR_HPP
#define SFA_STATE_VECTOR_HPP

#include "sfa/statevec/Element.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class StateVector final
{
public:

    struct ElementInfo
    {
        const char* name;
        IElement* elem;
    };

    struct Config
    {
        ElementInfo* elems;
    };

    StateVector(const Config kConfig);

    template<typename T>
    Result getElement(const char* const kName, Element<T>& kRet);

private:

    Config mConfig;

    Result getElementIndex(const char* const kName, U32& kRet) const;
};

#endif
