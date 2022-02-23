#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/Expression.hpp"
#include "sfa/core/Element.hpp"

class StateMachine final
{
public:

    struct Block final
    {
        IExpr<bool>* guard;
        Block* ifBlock;
        Block* elseBlock;
        IAction* action;
        Block* next;
    };

    struct StateConfig final
    {
        U32 id;
        Block* entry;
        Block* step;
        Block* exit;
    };

    struct Config final
    {
        Element<U32>& elemState;
        Element<U64>& elemStateTime;
        Element<U64>& elemGlobalTime;
        StateConfig* states;
    };

private:
};

#endif
