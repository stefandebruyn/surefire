////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Task.hpp"

ITask::ITask(StateVector& kSv, const Element<U8>* const kModeElem) :
    mSv(kSv), mModeElem(kModeElem), mInit(false)
{
}

Result ITask::init()
{
    // Check that task is not already initialized.
    if (mInit)
    {
        return E_TSK_REINIT;
    }

    // Call initialization implementation.
    const Result res = this->initImpl();
    if (res == SUCCESS)
    {
        mInit = true;
    }

    return res;
}

Result ITask::step()
{
    // Check that the task initialized successfully.
    if (!mInit)
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
    }

    // Invalid mode.
    return E_TSK_MODE;
}

Result ITask::stepSafe()
{
    return SUCCESS;
}
