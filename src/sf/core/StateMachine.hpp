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
/// @file  sf/core/StateMachine.hpp
/// @brief State machine object.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_STATE_MACHINE_HPP
#define SF_STATE_MACHINE_HPP

#include "sf/core/Action.hpp"
#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/core/Expression.hpp"
#include "sf/core/ExpressionStats.hpp"
#include "sf/core/Result.hpp"

///
/// @brief Implements a deterministic finite state machine that interfaces with
/// StateVector elements.
///
/// A state machine is a set of states, each with three logic "blocks": an
/// entry block that executes at the start of a state, a step block that
/// executes every step in the state, and an exit block that executes at the end
/// of the state. Blocks are represented as syntax trees of conditionals
/// ("guards") and actions. An action may be a state vector element assignment
/// or a transition to another state. All the data which a state machine
/// operates on are state vector elements.
///
/// @remark The user is not meant to manually create a StateMachine; it should
/// be the product of an autocoder of compiler in the framework config library.
///
/// @see IAction
/// @see IExprNode
/// @see IExpressionStats
///
class StateMachine final
{
public:

    ///
    /// @brief Reserved state ID which represents no state.
    ///
    static constexpr U32 NO_STATE = 0;

    ///
    /// @brief A node in a state machine syntax tree.
    ///
    /// Block execution goes as follows:
    ///
    ///   1. If guard is non-null, evaluate guard
    ///     1. If guard was true, execute if block
    ///     2. If guard was false and else block is non-null, execute else block
    ///   2. If action is non-null, execute action
    ///   3. If next block is non-null, execute next block
    ///
    struct Block final
    {
        ///
        /// @brief Pointer to block guard, or null if none.
        ///
        IExprNode<bool>* guard;

        ///
        /// @brief If block is guarded, pointer to if branch root block.
        ///
        Block* ifBlock;

        ///
        /// @brief If block is guarded, pointer to else branch root block, or
        /// null if none.
        ///
        Block* elseBlock;

        ///
        /// @brief Pointer to block action, or null if none.
        ///
        IAction* action;

        ///
        /// @brief Pointer to next block, or null if none.
        ///
        Block* next;

        ///
        /// @brief Recursively executes the block tree rooted at this block.
        ///
        /// @return Destination state if an action in the block tree triggered
        /// a state machine transition, otherwise StateMachine::NO_STATE.
        ///
        U32 execute() const;
    };

    ///
    /// @brief Configuration for a state machine state.
    ///
    struct StateConfig final
    {
        ///
        /// @brief Unique state ID.
        ///
        /// @note ID 0 is reserved.
        ///
        /// @see StateMachine::NO_STATE
        ///
        U32 id;

        ///
        /// @brief Pointer to entry block, or null if no entry block.
        ///
        Block* entry;

        ///
        /// @brief Pointer to step block, or null if no step block.
        ///
        Block* step;

        ///
        /// @brief Pointer to exit block, or null if no exit block.
        /// Transitioning in an exit block is illegal.
        ///
        Block* exit;
    };

    ///
    /// @brief State machine configuration.
    ///
    struct Config final
    {
        ///
        /// @brief Pointer to element which the state machine will write the
        /// current state ID in. The value of this element at the time of state
        /// machine initialization determines the initial state.
        ///
        /// @note This element should be read-only to external code, but writing
        /// it will not impact the state machine behavior.
        ///
        Element<U32>* elemState;

        ///
        /// @brief Pointer to element which the state machine will write the
        /// state elapsed time in. The unit of this element is the same as
        /// elemGlobalTime.
        ///
        /// @note This element should be read-only to external code, but writing
        /// it will not impact the state machine behavior.
        ///
        Element<U64>* elemStateTime;

        ///
        /// @brief Pointer to element storing the value of the global clock
        /// which the state machine uses. This element should be written by
        /// external code and must be monotonically-increasing across all state
        /// machine steps. The unit of time is implementation-dependent.
        ///
        Element<U64>* elemGlobalTime;

        ///
        /// @brief Array of state configs. The array must be terminated with a
        /// null (all-zero) state config.
        ///
        /// @warning Failing to null-terminate the array has undefined behavior.
        ///
        StateConfig* states;

        ///
        /// @brief Array of pointers to objects which compute statistics used
        /// by expressions in the state machine logic, or null if unused. The
        /// array must be terminated with a null pointer. Each state machine
        /// step, after updating the state and state time elements but before
        /// executing any blocks, the state machine will invoke update() on each
        /// object in the array.
        ///
        IExpressionStats** stats;
    };

    ///
    /// @brief Initializes a state machine from a config.
    ///
    /// @warning A StateMachine exists separately from the config. The config is
    /// not copied. The config and all data therein must live at least as long
    /// as the StateMachine. Modifying the config after using it to initialize a
    /// StateMachine has undefined behavior. The same config should not be used
    /// to initialize more than one StateMachine.
    ///
    /// @param[in]  kConfig  State machine config.
    /// @param[out] kSm      State machine to initialize.
    ///
    /// @retval SUCCESS       Successfully initialized state machine.
    /// @retval E_SM_REINIT   State machine is already initialized.
    /// @retval E_SM_NULL     A required pointer in the config was null.
    /// @retval E_SM_EMPTY    Config contains no states.
    /// @retval E_SM_STATE    Invalid initial state.
    /// @retval E_SM_TR_EXIT  Illegal transition in exit block.
    /// @retval E_SM_TRANS    Invalid transition destination state.
    ///
    static Result init(const Config kConfig, StateMachine& kSm);

    ///
    /// @brief Default constructor.
    ///
    /// @post The constructed StateMachine is uninitialized and invoking any of
    /// its methods returns an error.
    ///
    StateMachine();

    ///
    /// @brief Executes 1 cycle of state machine logic. If a state transition
    /// occurs this step, the new state will start on the next step (including
    /// updating the state element).
    ///
    /// @retval SUCCESS    Successfully stepped state machine.
    /// @retval E_SM_TIME  Global time value has not increased since last step.
    ///
    Result step();

    ///
    /// @brief Gets the value that will be written to the state elapsed time
    /// element on the next state machine step.
    ///
    /// @note This method is used when running the state machine in a state
    /// script runtime (see StateScriptAssembly). The user should have no reason
    /// to call it.
    ///
    /// @param[out] kT  Reference to assign state time.
    ///
    /// @retval SUCCESS  Successfully got state time.
    /// @retval [other]  Assertion failure.
    ///
    Result getStateTime(U64& kT) const;

    ///
    /// @brief Gets the value that will be written to the state element on the
    /// next state machine step. This value will differ from the state element's
    /// current value following a step which triggered a state transition; the
    /// state element will contain the ID of the state transitioned from, but
    /// this method will return the ID of the state transitioned to.
    ///
    /// @note This method is used when running the state machine in a state
    /// script runtime (see StateScriptAssembly). The user should have no reason
    /// to call it.
    ///
    /// @return Current state ID.
    ///
    U32 currentState() const;

    ///
    /// @brief Forcibly sets the state machine state, disregarding transitions
    /// and exit blocks. The next call to step() will execute the first step of
    /// the new state.
    ///
    /// @note This method is used when running the state machine in a state
    /// script runtime (see StateScriptAssembly). The user should never call it.
    ///
    /// @param[in] kStateId  Destination state ID.
    ///
    /// @retval SUCCESS  Successfully set state.
    /// @retval [other]  Assertion failure.
    ///
    Result setState(const U32 kStateId);

    StateMachine(const StateMachine&) = delete;
    StateMachine(StateMachine&&) = delete;
    StateMachine& operator=(const StateMachine&) = delete;
    StateMachine& operator=(StateMachine&&) = delete;

private:

    ///
    /// @brief State machine config.
    ///
    StateMachine::Config mConfig;

    ///
    /// @brief Pointer to current state config. When this is null, the state
    /// machine is uninitialized; the state machine factory method initializes
    /// the state machine by setting this pointer.
    ///
    StateMachine::StateConfig* mStateCur;

    ///
    /// @brief Global time value on the first step of the current state, or
    /// Clock::NO_TIME if the first step has not occurred yet.
    ///
    U64 mTimeStateStart;

    ///
    /// @brief Global time value on the last step.
    ///
    U64 mTimeLastStep;

    ///
    /// @brief Helper function to validate transitions in a state machine
    /// config.
    ///
    /// @param[in] kConfig  Config to validate.
    ///
    /// @retval SUCCESS       Transitions are valid.
    /// @retval E_SM_TR_EXIT  Illegal transition in exit block.
    /// @retval E_SM_TRANS    Invalid transition destination state.
    ///
    static Result checkTransitions(const StateMachine::Config kConfig);

    ///
    /// @brief Helper function to validate transitions in a state config.
    ///
    /// @param[in] kConfig  State machine config.
    /// @param[in] kBlock   Block to validate transitions in.
    /// @param[in] kExit    Whether block is an exit block.
    ///
    /// @retval SUCCESS       Transitions are valid.
    /// @retval E_SM_TR_EXIT  Illegal transition in exit block.
    /// @retval E_SM_TRANS    Invalid transition destination state.
    ///
    static Result checkBlockTransitions(const StateMachine::Config kConfig,
                                        const StateMachine::Block* const kBlock,
                                        const bool kExit);
};

#endif
