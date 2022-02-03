#ifndef SFA_IACTION_HPP
#define SFA_IACTION_HPP

#include "sfa/sm/ExpressionNode.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class IAction
{
public:

    const U32 destinationState;

    virtual ~IAction() = default;

    constexpr IAction(const IExpressionNode<bool>* const kGuard,
                      const U32 kDestState) :
        destinationState(kDestState), mGuard(kGuard)
    {
    }

    virtual Result evaluate(bool& kTransition) final;

    IAction(const IAction&) = delete;
    IAction(IAction&&) = delete;
    IAction& operator=(const IAction&) = delete;
    IAction& operator=(IAction&&) = delete;

protected:

    virtual Result execute(bool& kTransition) = 0;

private:

    const IExpressionNode<bool>* const mGuard;
};

#endif
