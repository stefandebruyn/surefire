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
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Task.hpp"

ITask::ITask(const Element<U8>* const kElemMode) :
    mModeElem(kElemMode), mInit(false)
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
