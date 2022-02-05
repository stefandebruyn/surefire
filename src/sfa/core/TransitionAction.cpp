#include "sfa/core/TransitionAction.hpp"

Result TransitionAction::execute(bool& kTransition)
{
    kTransition = true;
    return SUCCESS;
}
