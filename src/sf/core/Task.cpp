#include "sf/core/Task.hpp"

Result ITask::initialize()
{
    // Check that task is not already initialized.
    if (mInitialized)
    {
        return E_TSK_REINIT;
    }

    const Result res = this->initializeImpl();
    if (res == SUCCESS)
    {
        mInitialized = true;
    }
    return res;
}

Result ITask::step()
{
    // Check that the task initialized successfully.
    if (mInitialized != true)
    {
        return E_TSK_UNINIT;
    }

    // If a mode element was not provided, step in enabled mode.
    if (mModeElem == nullptr)
    {
        return this->stepEnable();
    }

    // Read mode element and invoke the corresponding step.
    const I8 mode = mModeElem->read();
    switch (mode)
    {
        case TaskMode::DISABLE:
            // Do nothing.
            return SUCCESS;

        case TaskMode::SAFE:
            return this->stepSafe();

        case TaskMode::ENABLE:
            return this->stepEnable();

        default:
            // Invalid mode.
            return E_TSK_MODE;
    }
}

Result ITask::stepSafe()
{
    return SUCCESS;
}
