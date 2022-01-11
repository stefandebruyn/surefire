#ifndef SFA_TRANSITION_ACTION_HPP
#define SFA_TRANSITION_ACTION_HPP

#include "sfa/sm/IAction.hpp"

class TransitionAction final : public IAction
{
public:

    constexpr TransitionAction(const IExpressionNode<bool>* const kGuard,
                               const U32 kDestState) :
        IAction(kGuard, kDestState)
    {
    }

    TransitionAction(const TransitionAction&) = delete;
    TransitionAction(TransitionAction&&) = delete;
    TransitionAction& operator=(const TransitionAction&) = delete;
    TransitionAction& operator=(TransitionAction&&) = delete;

private:

    Result execute(bool& kTransition) final override;
};

#endif
