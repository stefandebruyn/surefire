#include "sfa/sm/StateMachine.hpp"

const U64 StateMachine::mNoTime = 0xFFFFFFFFFFFFFFFF;

const U32 StateMachine::NO_STATE = 0xFFFFFFFF;

Result StateMachine::create(Config kConfig, StateMachine& kSm)
{
    Result res = E_UNREACHABLE;
    kSm = StateMachine(kConfig, res);
    return res;
}

StateMachine::StateMachine() :
    mConfig({nullptr, nullptr}),
    mTimeStateStart(mNoTime),
    mTimeLastStep(mNoTime),
    mCurrentState(nullptr)
{
}

StateMachine::StateMachine(Config kConfig, Result& kRes) : StateMachine()
{
    mConfig = kConfig;
    kRes = SUCCESS;

    // Check that state array and element are non-null.
    if ((mConfig.states == nullptr) || (mConfig.eState == nullptr))
    {
        kRes = E_NULLPTR;
        return;
    }
    
    // Check that initial state exists.
    const U32 initStateId = mConfig.eState->read();
    StateConfig* initStateConfig = nullptr;
    kRes = this->findState(initStateId, initStateConfig);
    if (kRes != SUCCESS)
    {
        return;
    }

    // Check state configs.
    U32 stateIdx = 0;
    while (mConfig.states[stateIdx].labels != nullptr)
    {
        const StateConfig* const stateConfig = &mConfig.states[stateIdx];

        // Check that state ID is not a reserved value.
        if (stateConfig->id == NO_STATE)
        {
            kRes = E_RESERVED;
            return;
        }

        // Check that state ID is unique.
        for (U32 i = 0; i < stateIdx; i++)
        {
            if (mConfig.states[i].id == stateConfig->id)
            {
                kRes = E_DUPLICATE;
                return;
            }
        }

        // Check label configs.
        U32 labelIdx = 0;
        while (stateConfig->labels[labelIdx].type != LAB_NULL)
        {
            const LabelConfig* const labelConfig =
                &stateConfig->labels[labelIdx];

            // Check that label type is valid.
            if (labelConfig->type >= LAB_LAST)
            {
                kRes = E_ENUM;
                return;
            }

            // If a range label, check that the range is valid.
            if ((labelConfig->type == LAB_RANGE)
                && (labelConfig->rangeUpper < labelConfig->rangeLower))
            {
                kRes = E_RANGE;
                return;
            }

            // If an exit label, check that no actions are transitions.
            if ((labelConfig->type == LAB_EXIT)
                && (labelConfig->actions != nullptr))
            {
                U32 actionIdx = 0;
                while (labelConfig->actions[actionIdx] != nullptr)
                {
                    if (labelConfig->actions[actionIdx]->destinationState
                        != NO_STATE)
                    {
                        kRes = E_TRANSITION;
                        return;
                    }
                }
            }

            // We don't check that the actions array is non-null since this can
            // represents a label with no actions.

            ++labelIdx;
        }

        ++stateIdx;
    }

    // If we got this far, the config is valid- set `mCurrentState` to the
    // initial state so that the state machine is usable.
    mCurrentState = initStateConfig;
}

Result StateMachine::step(const U64 kT)
{
    // Check that the state machine is initialized.
    if (mCurrentState == nullptr)
    {
        return E_UNINITIALIZED;
    }

    // Check that time is monotonically increasing.
    if ((mTimeLastStep != mNoTime) && (kT <= mTimeLastStep))
    {
        return E_TIME;
    }
    mTimeLastStep = kT;

    if (mTimeStateStart == mNoTime)
    {
        mTimeStateStart = kT;
    }

    // Execute actions for the current state.
    U32 i = 0;
    Result res = SUCCESS;
    U32 destState = NO_STATE;
    while (mCurrentState->labels[i].type != LAB_NULL)
    {
        LabelConfig* const label = &mCurrentState->labels[i];

        switch (label->type)
        {
            case LAB_ENTRY:
                // Entry actions happen only on the first step in the state.
                if (mTimeStateStart == kT)
                {
                    res = this->executeLabel(label, destState);
                }
                break;

            case LAB_STEP:
                // Step actions happen on every step in the state.
                res = this->executeLabel(label, destState);
                break;

            case LAB_RANGE:
            {
                // Range actions happen only in steps in the specified range.
                const U64 elapsed = (kT - mTimeStateStart);
                if ((elapsed >= label->rangeLower)
                    && (elapsed <= label->rangeUpper))
                {
                    res = this->executeLabel(label, destState);
                }
                break;
            }

            case LAB_EXIT:
                // Exit actions happen prior to leaving the state. They are not
                // executed at this point in time.
                break;

            default:
                // Unreachable assuming the state machine config was validated
                // correctly.
                return E_UNREACHABLE;
        }

        if (res != SUCCESS)
        {
            return res;
        }

        // Immediately break the loop when a transition is triggered.
        if (destState != NO_STATE)
        {
            break;
        }

        ++i;
    }

    if (destState != NO_STATE)
    {
        // Transitioning to a new state- execute exit label for the current
        // state.
        i = 0;
        while (mCurrentState->labels[i].type != LAB_NULL)
        {
            LabelConfig* const label = &mCurrentState->labels[i];
            if (label->type == LAB_EXIT)
            {
                // Transitioning in an exit label is not legal. This call to
                // `executeLabel` will not return a destination state in `_`
                // assuming the state machine config was validated correctly.
                U32 _;
                Result res = this->executeLabel(label, _);
                if (res != SUCCESS)
                {
                    return res;
                }
            }
            ++i;
        }

        // Transition to destination state. The next state machine step will be
        // the first step in the new state.
        res = this->findState(destState, mCurrentState);
        if (res != SUCCESS)
        {
            // Unreachable assuming the state machine config was validated
            // correctly.
            return res;
        }
        mConfig.eState->write(destState);
    }

    return SUCCESS;
}

Result StateMachine::executeLabel(LabelConfig* const kLabel,
                                  U32& kDestState)
{
    U32 i = 0;
    Result res;
    while (kLabel->actions[i] != nullptr)
    {
        // Evaluate transition. It will be executed if its guard is met.
        bool transition = false;
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

Result StateMachine::findState(const U32 kId, StateConfig*& kState)
{
    U32 i = 0;
    while (mConfig.states[i].labels != nullptr)
    {
        if (mConfig.states[i].id == kId)
        {
            kState = &mConfig.states[i];
            return SUCCESS;
        }
        ++i;
    }

    return E_STATE;
}
