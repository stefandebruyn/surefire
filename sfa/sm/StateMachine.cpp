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

    if (mTimeStateStart == mNoTime)
    {
        mTimeStateStart = kT;
    }

    U32 i = 0;
    Result res = SUCCESS;
    while (mCurrentState->labels[i].type != LAB_NULL)
    {
        LabelConfig* const label = &mCurrentState->labels[i];
        switch (label->type)
        {
            case LAB_ENTRY:
                if (mTimeStateStart == kT)
                {
                    res = this->executeLabel(label);
                }
                break;

            case LAB_STEP:
                res = this->executeLabel(label);
                break;

            case LAB_RANGE:
            {
                const U64 elapsed = (kT - mTimeStateStart);
                if ((elapsed >= label->rangeLower)
                    && (elapsed <= label->rangeUpper))
                {
                    res = this->executeLabel(label);
                }
                break;
            }

            default:
                if (label->type != LAB_EXIT)
                {
                    res = E_ENUM;
                }
        }

        if (res != SUCCESS)
        {
            return res;
        }

        ++i;
    }

    return SUCCESS;
}

Result StateMachine::executeLabel(LabelConfig* const kLabel)
{
    U32 i = 0;
    Result res;
    while (kLabel->actions[i] != nullptr)
    {
        U32 transitionToState = NO_STATE;
        // res = kLabel->actions[i]->evaluate(transitionToState);
        if (res != SUCCESS)
        {
            return res;
        }

        if (transitionToState != NO_STATE)
        {
            res = this->findState(transitionToState, mCurrentState);
            if (res != SUCCESS)
            {
                mCurrentState = nullptr;
                return res;
            }
            mConfig.eState->write(transitionToState);
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
