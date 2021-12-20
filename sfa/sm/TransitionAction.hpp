#ifndef SFA_TRANSITION_ACTION_HPP
#define SFA_TRANSITION_ACTION_HPP

#include "sfa/sm/IAction.hpp"

class TransitionAction final : public IAction
{
public:

    constexpr TransitionAction(const ExpressionTree<bool>* kGuard,
                               const U32 kTransitionState) :
        IAction(kGuard, kTransitionState)
    {
    }

    TransitionAction(const TransitionAction&) = delete;
    TransitionAction(TransitionAction&&) = delete;
    TransitionAction& operator=(const TransitionAction&) = delete;
    TransitionAction& operator=(TransitionAction&&) = delete;

protected:

    Result execute(bool& kTransition) final override;
};

#endif
