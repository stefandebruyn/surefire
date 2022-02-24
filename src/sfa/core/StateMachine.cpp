#include "sfa/core/StateMachine.hpp"
#include "sfa/pal/Clock.hpp"

U32 StateMachine::Block::execute()
{
    // Evaluate guard.
    if (guard != nullptr)
    {
        if (guard->evaluate() == true)
        {
            // Take if branch.
            if (ifBlock != nullptr)
            {
                const U32 ret = ifBlock->execute();
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
        else
        {
            // Take else branch.
            if (elseBlock != nullptr)
            {
                const U32 ret = elseBlock->execute();
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    // Execute action for this block.
    else if (action != nullptr)
    {
        const bool trans = action->execute();
        if (trans == true)
        {
            return action->destState;
        }
    }

    // Move to next block.
    if (next != nullptr)
    {
        return next->execute();
    }

    return 0;
}

Result StateMachine::create(const Config kConfig, StateMachine& kSm)
{
    if (kSm.mStateCur != nullptr)
    {
        return E_SM_REINIT;
    }

    if ((kConfig.elemState == nullptr)
        || (kConfig.elemStateTime == nullptr)
        || (kConfig.elemGlobalTime == nullptr)
        || (kConfig.states == nullptr))
    {
        return E_SM_NULL;
    }

    const U32 stateInit = kConfig.elemState->read();
    for (StateConfig* state = kConfig.states; state != nullptr; ++state)
    {
        if (state->id == stateInit)
        {
            kSm.mStateCur = state;
            break;
        }
    }

    if (kSm.mStateCur == nullptr)
    {
        return E_SM_STATE;
    }

    kSm.mConfig = kConfig;
    return SUCCESS;
}

StateMachine::StateMachine() :
    mConfig({nullptr, nullptr, nullptr, nullptr}),
    mStateCur(nullptr),
    mTimeStateStart(Clock::NO_TIME)
{
}

Result StateMachine::step()
{
    // Check that state machine is initialized.
    if (mStateCur == nullptr)
    {
        return E_SM_UNINIT;
    }

    // Compute time elapsed in current state.
    const U64 tCur = mConfig.elemGlobalTime->read();
    if (mTimeStateStart == Clock::NO_TIME)
    {
        mConfig.elemState->write(mStateCur->id);
        mTimeStateStart = tCur;
    }
    const U64 tStateElapsed = (tCur - mTimeStateStart);
    mConfig.elemStateTime->write(tStateElapsed);

    // Execute current state entry label.
    U32 destState = 0;
    if (tStateElapsed == 0)
    {
        if (mStateCur->entry != nullptr)
        {
            destState = mStateCur->entry->execute();
        }
    }

    // Execute current state step label if entry label did not transition.
    if ((destState != 0) && (mStateCur->step != nullptr))
    {
        destState = mStateCur->step->execute();
    }

    // If transitioning, do end-of-state logic.
    if (destState != 0)
    {
        // Execute current state exit label.
        if (mStateCur->exit != nullptr)
        {
            (void) mStateCur->exit->execute();
        }

        // Transition to new state.
        for (StateConfig* state = mConfig.states; state != nullptr; ++state)
        {
            if (state->id == destState)
            {
                mStateCur = state;
                mTimeStateStart = Clock::NO_TIME;
                break;
            }
        }
    }

    return SUCCESS;
}
