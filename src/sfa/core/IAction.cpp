#include "sfa/core/IAction.hpp"

Result IAction::evaluate(bool& kTransition)
{
    kTransition = false;
    bool execute = true;
    if (mGuard != nullptr)
    {
        const Result res = mGuard->evaluate(execute);
        if (res != SUCCESS)
        {
            return res;
        }
    }

    if (execute == true)
    {
        return this->execute(kTransition);
    }

    return SUCCESS;
}
