////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Action.hpp"

IAction::IAction(const U32 kDestState) : destState(kDestState)
{
}

IAssignmentAction::IAssignmentAction(const U32 kDestState) : IAction(kDestState)
{
}

TransitionAction::TransitionAction(const U32 kDestState) :
    IAction(kDestState)
{
}

bool TransitionAction::execute()
{
    return true;
}
