////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/Action.hpp
/// @brief State machine action objects.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_ACTION_HPP
#define SF_ACTION_HPP

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/Result.hpp"

///
/// @brief Abstract interface for a state machine action. Transitions are
/// considered special actions.
///
class IAction
{
public:

    ///
    /// @brief ID of destination state if the action is a transition, otherwise
    /// StateMachine::NO_STATE.
    ///
    const U32 destState;

    ///
    /// @brief Destructor.
    ///
    virtual ~IAction() = default;

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kDestState  Destination state ID or StateMachine::NO_STATE.
    ///
    IAction(const U32 kDestState);

    ///
    /// @brief Executes the action.
    ///
    /// @return Whether the action triggered a transition.
    ///
    virtual bool execute() = 0;

    IAction(const IAction&) = delete;
    IAction(IAction&&) = delete;
    IAction& operator=(const IAction&) = delete;
    IAction& operator=(IAction&&) = delete;
};

///
/// @brief Action which evaluates an expression and writes the value to a state
/// vector element.
///
/// @tparam T  Type of element and expression evaluation.
///
template<typename T>
class AssignmentAction final : public IAction
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kElem  Assignment LHS element.
    /// @param[in] kExpr  Assignment RHS expression.
    ///
    AssignmentAction(Element<T>& kElem, IExprNode<T>& kExpr) :
        IAction(0), mElem(kElem), mExpr(kExpr)
    {
    }

    ///
    /// @brief Evaluates the RHS and assigns it to the LHS.
    ///
    /// @return False.
    ///
    bool execute() final override
    {
        mElem.write(mExpr.evaluate());
        return false;
    }

private:

    ///
    /// @brief Assignment LHS element.
    ///
    Element<T>& mElem;

    ///
    /// @brief Assignment RHS expression.
    ///
    IExprNode<T>& mExpr;
};

///
/// @brief Action which triggers a state machine transition.
///
class TransitionAction final : public IAction
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kDestState  Destination state ID.
    ///
    TransitionAction(const U32 kDestState);

    ///
    /// @brief Triggers transition by simply returning true.
    ///
    /// @return True.
    ///
    bool execute() final override;
};

#endif
