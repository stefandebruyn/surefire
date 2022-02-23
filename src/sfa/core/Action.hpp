#ifndef SFA_ACTION_HPP
#define SFA_ACTION_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/Element.hpp"
#include "sfa/core/Expression.hpp"

class IAction
{
public:

    const U32 destState;

    IAction(const U32 kDestState) : destState(kDestState)
    {
    }

    virtual bool execute() = 0;
};

template<typename T>
class AssignmentAction final : public IAction
{
public:

    AssignmentAction(Element<T>& kElem, IExpr<T>& kExpr) :
        IAction(0), mElem(kElem), mExpr(kExpr)
    {
    }

    bool execute() final override
    {
        mElem.write(mExpr.evaluate());
        return false;
    }

private:

    Element<T>& mElem;

    IExpr<T>& mExpr;
};

class TransitionAction final : public IAction
{
public:

    TransitionAction(const U32 kDestState);

    bool execute() final override;
};

#endif
