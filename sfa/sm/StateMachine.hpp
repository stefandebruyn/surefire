#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sm/IAction.hpp"
#include "sfa/Result.hpp"

class StateMachine final
{
public:

    static const U32 NO_STATE;

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

    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    Result step(const U64 kT);

private:

    static const U64 mNoTime;

    Config mConfig;

    U64 mTimeStateStart;

    U64 mTimeLastStep;

    StateConfig* mCurrentState;

    StateMachine(Config kConfig, Result& kRes);

    StateMachine& operator=(StateMachine&&) = default;

    Result executeLabel(LabelConfig* const kLabel,
                        U32& kDestState);

    Result findState(const U32 kId, StateConfig*& kState);
};

#endif
