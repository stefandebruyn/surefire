#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"
#include "sfa/core/Expression.hpp"
#include "sfa/core/Element.hpp"
#include "sfa/core/Action.hpp"

class StateMachine final
{
public:

    static constexpr U32 NO_STATE = 0;

    struct Block final
    {
        IExprNode<bool>* guard;
        Block* ifBlock;
        Block* elseBlock;
        IAction* action;
        Block* next;

        U32 execute();
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
        Element<U32>* elemState;
        Element<U64>* elemStateTime;
        Element<U64>* elemGlobalTime;
        StateConfig* states;
    };

    static Result create(const Config kConfig, StateMachine& kSm);

    StateMachine();

    Result step();

private:

    Config mConfig;

    StateConfig* mStateCur;

    U64 mTimeStateStart;

    U64 mTimeLastStep;
};

#endif
