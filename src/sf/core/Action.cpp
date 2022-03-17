#include "sf/core/Action.hpp"

TransitionAction::TransitionAction(const U32 kDestState) :
    IAction(kDestState)
{
}

bool TransitionAction::execute()
{
    return true;
}
