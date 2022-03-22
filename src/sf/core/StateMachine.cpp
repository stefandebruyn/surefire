#include "sf/core/Assert.hpp"
#include "sf/core/StateMachine.hpp"
#include "sf/pal/Clock.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

static Result checkBlockTransitions(const StateMachine::Config kConfig,
                                    const StateMachine::Block* const kBlock,
                                    const bool kExit)
{
    // Base case: block is null.
    if (kBlock == nullptr)
    {
        return SUCCESS;
    }

    if ((kBlock->action != nullptr)
        && (kBlock->action->destState != StateMachine::NO_STATE))
    {
        // Block contains a transition action.

        if (kExit)
        {
            // Transitioning in an exit label is illegal.
            return E_SM_TR_EXIT;
        }

        // Find config of transition destination state.
        const StateMachine::StateConfig* state = kConfig.states;
        for (; state->id != StateMachine::NO_STATE; ++state)
        {
            if (state->id == kBlock->action->destState)
            {
                break;
            }
        }

        if (state->id == StateMachine::NO_STATE)
        {
            // Destination state not found.
            return E_SM_TRANS;
        }
    }

    // Recurse into linked blocks.
    Result res = checkBlockTransitions(kConfig, kBlock->ifBlock, kExit);
    if (res != SUCCESS)
    {
        return res;
    }
    res = checkBlockTransitions(kConfig, kBlock->elseBlock, kExit);
    if (res != SUCCESS)
    {
        return res;
    }
    res = checkBlockTransitions(kConfig, kBlock->next, kExit);
    if (res != SUCCESS)
    {
        return res;
    }

    return SUCCESS;
}

static Result checkTransitions(const StateMachine::Config kConfig)
{
    for (StateMachine::StateConfig* state = kConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        // Check transitions in entry label.
        Result res = checkBlockTransitions(kConfig, state->entry, false);
        if (res != SUCCESS)
        {
            return res;
        }

        // Check transitions in step label.
        res = checkBlockTransitions(kConfig, state->step, false);
        if (res != SUCCESS)
        {
            return res;
        }

        // Check transitions in exit label.
        res = checkBlockTransitions(kConfig, state->exit, true);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

/////////////////////////////////// Public /////////////////////////////////////

U32 StateMachine::Block::execute()
{
    // Evaluate guard.
    if (guard != nullptr)
    {
        if (guard->evaluate())
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
    if (action != nullptr)
    {
        const bool trans = action->execute();
        if (trans)
        {
            return action->destState;
        }
    }

    // Execute next block.
    if (next != nullptr)
    {
        return next->execute();
    }

    return StateMachine::NO_STATE;
}

Result StateMachine::create(const Config kConfig, StateMachine& kSm)
{
    // Check that state machine is not already initialized.
    if (kSm.mStateCur != nullptr)
    {
        return E_SM_REINIT;
    }

    // Check that none of the pointers are null.
    if ((kConfig.elemState == nullptr)
        || (kConfig.elemStateTime == nullptr)
        || (kConfig.elemGlobalTime == nullptr)
        || (kConfig.states == nullptr))
    {
        return E_SM_NULL;
    }

    // Check that config contains at least 1 state. This also serves to iterate
    // over the full state config array and partially verify it is well-formed.
    U32 numStates = 0;
    for (StateConfig* state = kConfig.states;
         state->id != StateMachine::NO_STATE;
         ++state)
    {
        ++numStates;
    }
    if (numStates == 0)
    {
        return E_SM_EMPTY;
    }

    // Find initial state based on state element.
    const U32 stateInit = kConfig.elemState->read();
    StateConfig* stateInitConfig = kConfig.states;
    for (; stateInitConfig->id != StateMachine::NO_STATE; ++stateInitConfig)
    {
        if (stateInitConfig->id == stateInit)
        {
            break;
        }
    }

    if (stateInitConfig->id == StateMachine::NO_STATE)
    {
        // Initial state not found.
        return E_SM_STATE;
    }

    // Check that all transitions are valid.
    const Result res = checkTransitions(kConfig);
    if (res != SUCCESS)
    {
        return res;
    }

    // Config is valid- assign state machine members so that the interface is
    // usable.
    kSm.mConfig = kConfig;
    kSm.mStateCur = stateInitConfig;

    return SUCCESS;
}

StateMachine::StateMachine() :
    mConfig({nullptr, nullptr, nullptr, nullptr, nullptr}),
    mStateCur(nullptr),
    mTimeStateStart(Clock::NO_TIME),
    mTimeLastStep(Clock::NO_TIME)
{
}

Result StateMachine::step()
{
    // Check that state machine is initialized.
    if (mStateCur == nullptr)
    {
        return E_SM_UNINIT;
    }

    SF_SAFE_ASSERT(mConfig.elemState != nullptr);
    SF_SAFE_ASSERT(mConfig.elemStateTime != nullptr);
    SF_SAFE_ASSERT(mConfig.elemGlobalTime != nullptr);
    SF_SAFE_ASSERT(mConfig.states != nullptr);

    // Check that the global time is valid and monotonic.
    const U64 tCur = mConfig.elemGlobalTime->read();
    if ((tCur == Clock::NO_TIME)
        || ((mTimeLastStep != Clock::NO_TIME) && (tCur <= mTimeLastStep)))
    {
        return E_SM_TIME;
    }

    // Compute time elapsed in current state.
    if (mTimeStateStart == Clock::NO_TIME)
    {
        mConfig.elemState->write(mStateCur->id);
        mTimeStateStart = tCur;
    }
    const U64 tStateElapsed = (tCur - mTimeStateStart);
    mConfig.elemStateTime->write(tStateElapsed);

    // Update expression stats if provided.
    if (mConfig.stats != nullptr)
    {
        for (IExpressionStats** stats = mConfig.stats;
             *stats != nullptr;
             ++stats)
        {
            (*stats)->update();
        }
    }

    // Execute current state entry label.
    U32 destState = StateMachine::NO_STATE;
    if (tStateElapsed == 0)
    {
        if (mStateCur->entry != nullptr)
        {
            destState = mStateCur->entry->execute();
        }
    }

    // Execute current state step label if entry label did not transition.
    if ((destState == StateMachine::NO_STATE) && (mStateCur->step != nullptr))
    {
        destState = mStateCur->step->execute();
    }

    // If transitioning, do end of state logic.
    if (destState != StateMachine::NO_STATE)
    {
        // Execute current state exit label.
        if (mStateCur->exit != nullptr)
        {
            (void) mStateCur->exit->execute();
        }

        // Transition to new state.
        for (StateConfig* state = mConfig.states;
             state->id != StateMachine::NO_STATE;
             ++state)
        {
            if (state->id == destState)
            {
                mStateCur = state;
                mTimeStateStart = Clock::NO_TIME;
                break;
            }
        }

        // Assert that the destination state was found.
        SF_SAFE_ASSERT(mStateCur->id == destState);
    }

    // Update last step time.
    mTimeLastStep = tCur;

    return SUCCESS;
}
