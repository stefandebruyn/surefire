#include "sfa/core/ITask.hpp"

Result ITask::initialize()
{
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
        return E_UNINITIALIZED;
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
        case MODE_DISABLE:
            // Do nothing.
            return SUCCESS;

        case MODE_SAFE:
            return this->stepSafe();

        case MODE_ENABLE:
            return this->stepEnable();

        default:
            // Invalid mode.
            return E_ENUM;
    }
}

Result ITask::stepSafe()
{
    return SUCCESS;
}
