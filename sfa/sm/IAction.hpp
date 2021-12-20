#ifndef SFA_IACTION_HPP
#define SFA_IACTION_HPP

#include "sfa/sv/StateVector.hpp"
#include "sfa/sm/ExpressionTree.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class IAction
{
public:

    const U32 transitionState;

    virtual ~IAction() = default;

    constexpr IAction(StateVector& kStateVector,
                      const ExpressionTree<bool>* kGuard,
                      const U32 kTransitionState) :
        mStateVector(kStateVector),
        mGuard(kGuard),
        transitionState(kTransitionState)
    {
    }

    virtual Result evaluate(bool& kTransition) final;

protected:

    StateVector& mStateVector;

    virtual Result execute(bool& kTransition) = 0;

private:

    const ExpressionTree<bool>* mGuard;
};

#endif
