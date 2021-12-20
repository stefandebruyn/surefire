#ifndef SFA_IACTION_HPP
#define SFA_IACTION_HPP

#include "sfa/sv/StateVector.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class IAction
{
public:

    virtual ~IAction() = default;

    constexpr IAction(StateVector* kStateVector) : mStateVector(kStateVector)
    {
    }

    virtual Result evaluate(U32& kTransitionToState) = 0;

    virtual bool canTransition(U32& kTargetState) = 0;

protected:

    StateVector* mStateVector;

    virtual Result execute(U32& kTransitionToState) = 0;
};

#endif
