#ifndef SF_STATE_MACHINE_HPP
#define SF_STATE_MACHINE_HPP

#include "sf/core/Action.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/Result.hpp"

class StateMachine final
{
public:

    static constexpr U32 NO_STATE = 0;

    struct Block final
    {
        const IExprNode<bool>* guard;
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
