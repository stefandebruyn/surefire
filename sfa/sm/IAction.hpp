#ifndef SFA_IACTION_HPP
#define SFA_IACTION_HPP

#include "sfa/sm/ExpressionTree.hpp"
#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class IAction
{
public:

    const U32 destinationState;

    virtual ~IAction() = default;

    constexpr IAction(const ExpressionTree<bool>* kGuard,
                      const U32 kDestState) :
        mGuard(kGuard), destinationState(kDestState)
    {
    }

    IAction(const IAction&) = delete;
    IAction(IAction&&) = delete;
    IAction& operator=(const IAction&) = delete;
    IAction& operator=(IAction&&) = delete;

    virtual Result evaluate(bool& kTransition) final;

protected:

    virtual Result execute(bool& kTransition) = 0;

private:

    const ExpressionTree<bool>* mGuard;
};

#endif
