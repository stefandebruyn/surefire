#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sm/IAction.hpp"
#include "sfa/Result.hpp"

class StateMachine final
{
public:

    enum LabelType : U32
    {
        LAB_NULL,
        LAB_ENTRY,
        LAB_STEP,
        LAB_RANGE,
        LAB_EXIT,
        LAB_LAST
    };

    struct LabelConfig
    {
        LabelType type;
        IAction** actions;
        U64 rangeLower;
        U64 rangeUpper;
    };

    struct StateConfig
    {
        U32 id;
        LabelConfig* labels;
    };

    struct Config
    {
        StateConfig* states;
        Element<U32>* eState;
    };

    static Result create(Config kConfig, StateMachine& kSm);

    StateMachine();

    Result step(const U64 kT);

private:

    static const U64 mNoTime;

    static const U32 mNoState;

    Config mConfig;

    U64 mTimeStateStart;

    StateConfig* mCurrentState;

    StateMachine(Config kConfig, Result& kRes);

    Result executeLabel(LabelConfig* const kLabel);

    Result findState(const U32 kId, StateConfig*& kState);
};

#endif
