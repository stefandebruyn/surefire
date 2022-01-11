#ifndef SFA_ASSIGNMENT_ACTION_HPP
#define SFA_ASSIGNMENT_ACTION_HPP

#include "sfa/sm/StateMachine.hpp"
#include "sfa/sm/IAction.hpp"
#include "sfa/sv/Element.hpp"

template<typename T>
class AssignmentAction final : public IAction
{
public:

    constexpr AssignmentAction(const IExpressionNode<bool>* const kGuard,
                               Element<T>& kElem,
                               const IExpressionNode<T>& kExpr) :
        IAction(kGuard, StateMachine::NO_STATE), mElem(kElem), mExpr(kExpr)
    {
    }

    AssignmentAction(const AssignmentAction<T>&) = delete;
    AssignmentAction(AssignmentAction<T>&&) = delete;
    AssignmentAction<T>& operator=(const AssignmentAction<T>&) = delete;
    AssignmentAction<T>& operator=(AssignmentAction<T>&&) = delete;

private:

    Element<T>& mElem;

    const IExpressionNode<T>& mExpr;

    Result execute(bool& kTransition) final override
    {
        kTransition = false;
        T value = 0;
        const Result res = mExpr.evaluate(value);
        if (res != SUCCESS)
        {
            return res;
        }
        mElem.write(value);
        return SUCCESS;
    }
};

#endif
