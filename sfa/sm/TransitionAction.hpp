#ifndef SFA_TRANSITION_ACTION_HPP
#define SFA_TRANSITION_ACTION_HPP

#include "sfa/sm/IAction.hpp"

class TransitionAction final : public IAction
{
public:

    constexpr TransitionAction(StateVector& kStateVector,
                               const ExpressionTree<bool>* kGuard,
                               const U32 kTransitionState) :
        IAction(kStateVector, kGuard, kTransitionState)
    {
    }

protected:

    Result execute(bool& kTransition) final override;
};

#endif
