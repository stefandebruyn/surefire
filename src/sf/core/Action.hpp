#ifndef SF_ACTION_HPP
#define SF_ACTION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/Result.hpp"

class IAction
{
public:

    const U32 destState;

    virtual ~IAction() = default;

    IAction(const U32 kDestState) : destState(kDestState)
    {
    }

    virtual bool execute() = 0;
};

template<typename T>
class AssignmentAction final : public IAction
{
public:

    AssignmentAction(Element<T>& kElem, const IExprNode<T>& kExpr) :
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

    const IExprNode<T>& mExpr;
};

class TransitionAction final : public IAction
{
public:

    TransitionAction(const U32 kDestState);

    bool execute() final override;
};

#endif
