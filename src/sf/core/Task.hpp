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
/// @file  sf/core/Task.hpp
/// @brief Task interface.
////////////////////////////////////////////////////////////////////////////////

#ifndef SF_TASK_HPP
#define SF_TASK_HPP

#include "sf/core/Element.hpp"
#include "sf/core/Result.hpp"
#include "sf/core/StateVector.hpp"

///
/// @brief Enumeration of possible task execution modes.
///
enum TaskMode : U8
{
    DISABLE = 0,
    SAFE = 1,
    ENABLE = 2
};

///
/// @brief Abtract interface for a task.
///
/// A task is a unit of application business logic that repeats periodically
/// according to some schedule (though this interface does not capture the
/// scheduling aspect). The behavior of a task may optionally be controlled
/// through a state vector element representing the task's "mode". There are
/// three possible modes: disabled, safed, and enabled. The user decides the
/// meaning of these modes in the context of their particular application.
///
/// @remark Basic steps to implement a task:
///
///   (1) Define a class which publicly inherits from ITask
///   (2) Constructor takes the parameters required by the ITask constructor
///       along with anything else needed to configure the task
///   (3) Implement task initialization logic in initImpl(); this will
///       usually include grabbing pointers to state vector elements that the
///       task uses as inputs and outputs
///   (4) Implement task business logic in stepEnable() and optionally
///       stepSafe()
///
/// @remark Basic steps to run a task:
///
///   (1) Construct task
///   (2) Invoke initialize() on task
///   (3) Invoke step() on task at the desired frequency
///   (4) If the task was configured with a mode state vector element, change
///       the task mode by writing this element from other code
///
/// @remark Normally tasks will be used in conjunction with an IExecutor, which
/// automatically handles task initialization and execution. Additionally, a
/// StateMachine is a good mechanism for controlling task modes.
///
class ITask
{
public:

    ///
    /// @brief Constructor. The constructed task is initially uninitialized and
    /// invoking any methods on it will fail.
    ///
    /// @param[in] kElemMode  Read-only element controlling task mode, or null
    ///                       if not using modes, in which case the task always
    ///                       runs in enabled mode.
    ///
    ITask(const Element<U8>* const kElemMode);

    ///
    /// @brief Destructor.
    ///
    virtual ~ITask() = default;

    ///
    /// @brief Initializes the task by calling initImpl(). On SUCCESS, step()
    /// may be successfully invoked on the task.
    ///
    /// @retval SUCCESS       Task initialized successfully.
    /// @retval E_TSK_REINIT  Task is already initialized.
    /// @retval [other]       Error returned by the initialization
    ///                       implementation.
    ///
    virtual Result init() final;

    ///
    /// @brief Executes 1 cycle of task logic. If the mode element is set to
    /// TaskMode::ENABLE or no mode element was provided, stepEnable() is
    /// called. If the the mode element is set to TaskMode::SAFE, stepSafe()
    /// is called. If the mode element is set to TaskMode::DISABLE, nothing
    /// happens.
    ///
    /// @retval SUCCESS       Task successfully stepped.
    /// @retval E_TSK_UNINIT  Task is not initialized.
    /// @retval E_TSK_MODE    Mode element had an invalid value.
    /// @retval [other]       Error returned by the step implementation.
    ///
    virtual Result step() final;

    ITask(const ITask&) = delete;
    ITask(ITask&&) = delete;
    ITask& operator=(const ITask&) = delete;
    ITask& operator=(ITask&&) = delete;

protected:

    ///
    /// @brief One-time task initialization logic.
    ///
    /// @remark Tasks will normally take this opportunity to grab input and
    /// output element pointers from the state vector.
    ///
    /// @retval SUCCESS  Task initialized successfully.
    /// @retval [other]  Implemntation-defined.
    ///
    virtual Result initImpl() = 0;

    ///
    /// @brief Called when task steps in safe mode.
    ///
    /// @note Implementing this method is optional. It does nothing by default.
    ///
    /// @retval SUCCESS  Task stepped successfully.
    /// @retval [other]  Implementation-defined.
    ///
    virtual Result stepSafe();

    ///
    /// @brief Called when task steps in enable mode.
    ///
    /// @retval SUCCESS  Task stepped successfully.
    /// @retval [other]  Implementation-defined.
    ///
    virtual Result stepEnable() = 0;

private:

    ///
    /// @brief Task mode state vector element, or null if not using modes.
    ///
    const Element<U8>* const mModeElem;

    ///
    /// @brief Whether task has initialized.
    ///
    bool mInit;
};

#endif
