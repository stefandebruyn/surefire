#include "sfa/core/StateMachine.hpp"

const U64 StateMachine::mNoTime = 0xFFFFFFFFFFFFFFFF;

Result StateMachine::create(const Config kConfig, StateMachine& kSm)
{
    // Check that provided state machine is not already initialized.
    if (kSm.mCurrentState != nullptr)
    {
        return E_REINIT;
    }

    Result res = E_UNREACHABLE;
    kSm = StateMachine(kConfig, res);
    return res;
}

StateMachine::StateMachine() :
    mConfig({nullptr, nullptr, nullptr, nullptr}),
    mTimeStateStart(mNoTime),
    mTimeLastStep(mNoTime),
    mCurrentState(nullptr)
{
}

StateMachine::StateMachine(const Config kConfig, Result& kRes) : StateMachine()
{
    mConfig = kConfig;
    kRes = SUCCESS;

    // Check that state array, state element, and time element are non-null.
    if ((mConfig.states == nullptr)
        || (mConfig.elemState == nullptr)
        || (mConfig.elemGlobalTime == nullptr))
    {
        kRes = E_NULLPTR;
        return;
    }

    // Check that initial state exists.
    const U32 initStateId = mConfig.elemState->read();
    StateConfig* initStateConfig = nullptr;
    kRes = this->getStateConfig(initStateId, initStateConfig);
    if (kRes != SUCCESS)
    {
        return;
    }

    // Check state configs.
    for (U32 i = 0; mConfig.states[i].id != NO_STATE; ++i)
    {
        const StateConfig* const stateConfig = &mConfig.states[i];

        // Check that state ID is unique.
        for (U32 j = 0; j < i; j++)
        {
            if (mConfig.states[j].id == stateConfig->id)
            {
                kRes = E_DUPLICATE;
                return;
            }
        }

        // Check that transitions in entry and step labels are to valid states.
        kRes = this->validateLabelTransitions(stateConfig->entryLabel);
        if (kRes != SUCCESS)
        {
            return;
        }
        kRes = this->validateLabelTransitions(stateConfig->stepLabel);
        if (kRes != SUCCESS)
        {
            return;
        }

        // Check range labels.
        if (stateConfig->rangeLabels != nullptr)
        {
            for (U32 j = 0; stateConfig->rangeLabels[j].actions != nullptr; ++j)
            {
                const LabelConfig* const labelConfig =
                    &stateConfig->rangeLabels[j];

                // Check that range is valid.
                if (labelConfig->rangeLower > labelConfig->rangeUpper)
                {
                    kRes = E_RANGE;
                    return;
                }

                // Check that transitions are to valid states.
                kRes = this->validateLabelTransitions(*labelConfig);
                if (kRes != SUCCESS)
                {
                    return;
                }
            }
        }

        // If an exit label was provided, check that none of the actions can
        // trigger transitions.
        if (stateConfig->exitLabel.actions != nullptr)
        {
            for (U32 j = 0; stateConfig->exitLabel.actions[j] != nullptr; ++j)
            {
                if (stateConfig->exitLabel.actions[j]->destinationState
                    != NO_STATE)
                {
                    kRes = E_TRANSITION;
                    return;
                }
            }
        }
    }

    // If we got this far, the config is valid- set `mCurrentState` to the
    // initial state so that the state machine is usable.
    mCurrentState = initStateConfig;
}

Result StateMachine::step()
{
    // Check that the state machine is initialized.
    if (mCurrentState == nullptr)
    {
        return E_UNINITIALIZED;
    }

    // Check that time is monotonically increasing.
    const U64 curTime = mConfig.elemGlobalTime->read();
    if ((mTimeLastStep != mNoTime) && (curTime <= mTimeLastStep))
    {
        return E_TIME;
    }
    mTimeLastStep = curTime;

    // If the state start time is unset, this is the first step of the current
    // state.
    Result res = SUCCESS;
    U32 destState = NO_STATE;
    if (mTimeStateStart == mNoTime)
    {
        // Record the state start time.
        mTimeStateStart = curTime;

        // Update state element.
        mConfig.elemState->write(mCurrentState->id);

        // Execute state entry label.
        res = this->executeLabel(mCurrentState->entryLabel, destState);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // If a state time element was provided, write the state elapsed time to it.
    const U64 stateElapsedTime = (curTime - mTimeStateStart);
    if (mConfig.elemStateTime != nullptr)
    {
        mConfig.elemStateTime->write(stateElapsedTime);
    }

    // Execute step label for current state if a previous label did not trigger
    // a transition.
    if (destState == NO_STATE)
    {
        res = this->executeLabel(mCurrentState->stepLabel, destState);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Execute any range labels for current state if the current time is in
    // range and a previous label did not trigger a transition.
    if ((destState == NO_STATE) && (mCurrentState->rangeLabels != nullptr))
    {
        for (U32 i = 0; mCurrentState->rangeLabels[i].actions != nullptr; ++i)
        {
            const LabelConfig* const labelConfig =
                &mCurrentState->rangeLabels[i];

            if ((stateElapsedTime >= labelConfig->rangeLower)
                && (stateElapsedTime <= labelConfig->rangeUpper))
            {
                res = this->executeLabel(mCurrentState->rangeLabels[i],
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
        res = this->executeLabel(mCurrentState->exitLabel, _);

        // Transition to destination state. The next state machine step will be
        // the first step in the new state.
        res = this->getStateConfig(destState, mCurrentState);
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

Result StateMachine::executeLabel(const LabelConfig& kLabel, U32& kDestState)
{
    // A null actions array indicates an empty label.
    if (kLabel.actions == nullptr)
    {
        return SUCCESS;
    }

    bool transition = false;
    Result res = E_UNREACHABLE;
    for (U32 i = 0; kLabel.actions[i] != nullptr; ++i)
    {
        // Evaluate transition. It will be executed if its guard is met.
        res = kLabel.actions[i]->evaluate(transition);
        if (res != SUCCESS)
        {
            return res;
        }

        // If the action triggered a transition, immeditiately return the
        // destination state to the caller without executing more actions.
        if (transition == true)
        {
            kDestState = kLabel.actions[i]->destinationState;
            return SUCCESS;
        }
    }

    return SUCCESS;
}

Result StateMachine::getStateConfig(const U32 kId, StateConfig*& kState)
{
    for (U32 i = 0; mConfig.states[i].id != NO_STATE; ++i)
    {
        if (mConfig.states[i].id == kId)
        {
            kState = &mConfig.states[i];
            return SUCCESS;
        }
    }

    return E_STATE;
}

Result StateMachine::validateLabelTransitions(const LabelConfig& kLabel)
{
    if (kLabel.actions == nullptr)
    {
        return SUCCESS;
    }

    for (U32 i = 0; kLabel.actions[i] != nullptr; ++i)
    {
        const U32 destState = kLabel.actions[i]->destinationState;
        if (destState != NO_STATE)
        {
            StateConfig* _;
            const Result res = this->getStateConfig(destState, _);
            if (res != SUCCESS)
            {
                return res;
            }
        }
    }

    return SUCCESS;
}
