#include "sfa/sm/StateMachine.hpp"

const U64 StateMachine::mNoTime = 0xFFFFFFFFFFFFFFFF;

const U32 StateMachine::NO_STATE = 0;

Result StateMachine::create(const Config& kConfig, StateMachine& kSm)
{
    Result res = E_UNREACHABLE;
    kSm = StateMachine(kConfig, res);
    return res;
}

StateMachine::StateMachine() :
    mConfig(nullptr),
    mTimeStateStart(mNoTime),
    mTimeLastStep(mNoTime),
    mCurrentState(nullptr)
{
}

StateMachine::StateMachine(const Config& kConfig, Result& kRes) : StateMachine()
{
    kRes = SUCCESS;
    mConfig = &kConfig;

    // Check that state array and state element are non-null.
    if ((mConfig->states == nullptr) || (mConfig->eState == nullptr))
    {
        kRes = E_NULLPTR;
        return;
    }

    // Check that initial state exists.
    const U32 initStateId = mConfig->eState->read();
    const StateConfig* initStateConfig = nullptr;
    kRes = this->findState(initStateId, initStateConfig);
    if (kRes != SUCCESS)
    {
        return;
    }

    // Check state configs.
    for (U32 i = 0; mConfig->states[i].id != NO_STATE; ++i)
    {
        const StateConfig* const stateConfig = &mConfig->states[i];

        // Check that state ID is unique.
        for (U32 j = 0; j < i; j++)
        {
            if (mConfig->states[j].id == stateConfig->id)
            {
                kRes = E_DUPLICATE;
                return;
            }
        }

        // If range labels were provided, check that the ranges are valid.
        if (stateConfig->rangeLabels != nullptr)
        {
            for (U32 i = 0; stateConfig->rangeLabels[i].actions != nullptr; ++i)
            {
                const LabelConfig* const labelConfig =
                    &stateConfig->rangeLabels[i];
                if (labelConfig->rangeLower > labelConfig->rangeUpper)
                {
                    kRes = E_RANGE;
                    return;
                }
            }
        }

        // If an exit label was provided, check that none of the actions can
        // trigger transitions.
        if (stateConfig->exitLabel.actions != nullptr)
        {
            for (U32 i = 0; stateConfig->exitLabel.actions[i] != nullptr; ++i)
            {
                if (stateConfig->exitLabel.actions[i]->destinationState
                    != NO_STATE)
                {
                    kRes = E_TRANSITION;
                    return;
                }
            }
        }
    }

    // If we got this far, the config is valid- set current state so that the
    // state machine is usable.
    mCurrentState = initStateConfig;
}

Result StateMachine::step(const U64 kT)
{
    // Check that the state machine is initialized.
    if ((mCurrentState == nullptr) || (mConfig == nullptr))
    {
        return E_UNINITIALIZED;
    }

    // Check that time is monotonically increasing.
    if ((mTimeLastStep != mNoTime) && (kT <= mTimeLastStep))
    {
        return E_TIME;
    }
    mTimeLastStep = kT;

    // If the state start time is unset, this is the first step of the current
    // state.
    Result res = SUCCESS;
    U32 destState = NO_STATE;
    if (mTimeStateStart == mNoTime)
    {
        // Record the state start time.
        mTimeStateStart = kT;

        // Update state element.
        mConfig->eState->write(mCurrentState->id);

        // Execute state entry label.
        res = this->executeLabel(&mCurrentState->entryLabel, destState);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Execute step label for current state if and a previous label did not
    // trigger a transition.
    if (destState == NO_STATE)
    {
        res = this->executeLabel(&mCurrentState->stepLabel, destState);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Execute any range labels for current state if the current time is in
    // range and a previous label did not trigger a transition.
    if ((destState == NO_STATE) && (mCurrentState->rangeLabels != nullptr))
    {
        const U64 elapsed = (kT - mTimeStateStart);
        for (U32 i = 0; mCurrentState->rangeLabels[i].actions != nullptr; ++i)
        {
            const LabelConfig* const labelConfig =
                &mCurrentState->rangeLabels[i];

            if ((elapsed >= labelConfig->rangeLower)
                && (elapsed <= labelConfig->rangeUpper))
            {
                res = this->executeLabel(&mCurrentState->rangeLabels[i],
                                         destState);
                if (res != SUCCESS)
                {
                    return res;
                }
                if (destState != NO_STATE)
                {
                    break;
                }
            }
        }
    }

    // Do end-of-state logic if a previous label triggered a transition.
    if (destState != NO_STATE)
    {
        // Execute exit label. Transitioning in an exit label is illegal, so
        // this `executeLabel` call should not return a state in `_` assuming
        // the state machine config was validated correctly.
        U32 _;
        res = this->executeLabel(&mCurrentState->exitLabel, _);

        // Transition to destination state. The next state machine step will be
        // the first step in the new state.
        res = this->findState(destState, mCurrentState);
        if (res != SUCCESS)
        {
            // Unreachable assuming the state machine config was validated
            // correctly.
            return res;
        }

        // Reset state start time.
        mTimeStateStart = mNoTime;
    }

    return SUCCESS;
}

Result StateMachine::executeLabel(const LabelConfig* const kLabel,
                                  U32& kDestState)
{
    // A null actions array indicates an empty label.
    if (kLabel->actions == nullptr)
    {
        return SUCCESS;
    }

    bool transition = false;
    Result res = E_UNREACHABLE;
    for (U32 i = 0; kLabel->actions[i] != nullptr; ++i)
    {
        // Evaluate transition. It will be executed if its guard is met.
        res = kLabel->actions[i]->evaluate(transition);
        if (res != SUCCESS)
        {
            return res;
        }

        // If the action triggered a transition, immeditiately return the
        // destination state to the caller without executing more actions.
        if (transition == true)
        {
            kDestState = kLabel->actions[i]->destinationState;
            return SUCCESS;
        }
    }

    return SUCCESS;
}

Result StateMachine::findState(const U32 kId, const StateConfig*& kState)
{
    for (U32 i = 0; mConfig->states[i].id != NO_STATE; ++i)
    {
        if (mConfig->states[i].id == kId)
        {
            kState = &mConfig->states[i];
            return SUCCESS;
        }
    }

    return E_STATE;
}
