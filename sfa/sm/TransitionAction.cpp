#include "sfa/sm/TransitionAction.hpp"

Result TransitionAction::execute(bool& kTransition)
{
    kTransition = true;
    return SUCCESS;
}
