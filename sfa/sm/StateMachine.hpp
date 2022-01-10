#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sm/IAction.hpp"
#include "sfa/Result.hpp"

class StateMachine final
{
public:

    static constexpr U32 NO_STATE = 0;

    struct LabelConfig final
    {
        IAction** actions;
        U64 rangeLower;
        U64 rangeUpper;
    };

    struct StateConfig final
    {
        U32 id;
        LabelConfig entryLabel;
        LabelConfig stepLabel;
        LabelConfig* rangeLabels;
        LabelConfig exitLabel;
    };

    struct Config final
    {
        StateConfig* states;
        Element<U32>* elemState;
        const Element<U64>* elemGlobalTime;
        Element<U64>* elemStateTime;
    };

    static Result create(const Config kConfig, StateMachine& kSm);

    StateMachine();

    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    Result step();

private:

    static const U64 mNoTime;

    Config mConfig;

    U64 mTimeStateStart;

    U64 mTimeLastStep;

    StateConfig* mCurrentState;

    StateMachine(const Config kConfig, Result& kRes);

    StateMachine& operator=(StateMachine&&) = default;

    Result executeLabel(const LabelConfig& kLabel, U32& kDestState);

    Result findState(const U32 kId, StateConfig*& kState);

    Result validateLabelTransitions(const LabelConfig& kLabel);
};

#endif
