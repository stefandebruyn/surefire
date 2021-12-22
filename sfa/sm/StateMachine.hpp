#ifndef SFA_STATE_MACHINE_HPP
#define SFA_STATE_MACHINE_HPP

#include "sfa/sv/Element.hpp"
#include "sfa/sm/IAction.hpp"
#include "sfa/Result.hpp"

class StateMachine final
{
public:

    static const U32 NO_STATE;

    struct LabelConfig
    {
        IAction* const* const actions;
        const U64 rangeLower;
        const U64 rangeUpper;
    };

    struct StateConfig
    {
        const U32 id;
        const LabelConfig entryLabel;
        const LabelConfig stepLabel;
        const LabelConfig* const rangeLabels;
        const LabelConfig exitLabel;
    };

    struct Config
    {
        const StateConfig* const states;
        Element<U32>* const eState;
    };

    static Result create(const Config& kConfig, StateMachine& kSm);

    StateMachine();

    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;

    Result step(const U64 kT);

private:

    static const U64 mNoTime;

    const Config* mConfig;

    U64 mTimeStateStart;

    U64 mTimeLastStep;

    const StateConfig* mCurrentState;

    StateMachine(const Config& kConfig, Result& kRes);

    StateMachine& operator=(StateMachine&&) = default;

    Result executeLabel(const LabelConfig* const kLabel,
                        U32& kDestState);

    Result findState(const U32 kId, const StateConfig*& kState);
};

#endif
