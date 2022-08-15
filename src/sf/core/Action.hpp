////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
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
/// @brief Abstract interface for the AssignmentAction template.
///
/// @remark This interface helps reduce boilerplate in an autocoder by
/// allowing access to AssignmentAction members without downcasting to a
/// specific instantiation of the AssignmentAction template.
///
class IAssignmentAction : public IAction
{
public:

    ///
    /// @see IAction::IAction(U32)
    ///
    IAssignmentAction(const U32 kDestState);

    ///
    /// @brief Gets the assignment LHS element.
    ///
    /// @return Assignment LHS element.
    ///
    virtual const IElement& elem() const = 0;

    ///
    /// @brief Gets the assignment RHS expression.
    ///
    /// @return Assignment RHS expression.
    ///
    virtual const IExpression& expr() const = 0;
};

///
/// @brief Action which evaluates an expression and writes the value to a state
/// vector element.
///
/// @tparam T  Type of element and expression evaluation.
///
template<typename T>
class AssignmentAction final : public IAssignmentAction
{
public:

    ///
    /// @brief Constructor.
    ///
    /// @param[in] kElem  Assignment LHS element.
    /// @param[in] kExpr  Assignment RHS expression.
    ///
    AssignmentAction(Element<T>& kElem, IExprNode<T>& kExpr) :
        IAssignmentAction(0), mElem(kElem), mExpr(kExpr)
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

    ///
    /// @see IAssignmentAction::elem()
    ///
    const IElement& elem() const final override
    {
        return static_cast<const IElement&>(mElem);
    }

    ///
    /// @see IAssignmentAction::expr()
    ///
    const IExpression& expr() const final override
    {
        return static_cast<const IExpression&>(mExpr);
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
