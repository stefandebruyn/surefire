#ifndef SFA_ASSIGNMENT_ACTION_HPP
#define SFA_ASSIGNMENT_ACTION_HPP

#include "sfa/sm/IAction.hpp"
#include "sfa/sv/Element.hpp"

template<typename T>
class AssignmentAction final : public IAction
{
public:

    constexpr AssignmentAction(StateVector& kStateVector,
                               const ExpressionTree<bool>* kGuard,
                               Element<T>& kElem,
                               const ExpressionTree<T>& kExpr) :
        IAction(kStateVector, kGuard, 0xFFFFFFFF), mElem(kElem), mExpr(kExpr)
    {
    }

protected:

    Result execute(bool& kTransition) final override
    {
        kTransition = false;
        T value = 0;
        Result res = mExpr.evaluate(value);
        if (res != SUCCESS)
        {
            return res;
        }
        mElem.write(value);
        return SUCCESS;
    }

private:

    Element<T>& mElem;

    const ExpressionTree<T>& mExpr;
};

#endif
