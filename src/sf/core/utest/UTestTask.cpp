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
/// @file  sf/core/utest/UTestTask.cpp
/// @brief Unit tests for the ITask interface.
////////////////////////////////////////////////////////////////////////////////

#include <cstring>

#include "sf/core/Task.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

// Test state vector backing storage.
#pragma pack(push, 1)
static struct
{
    U8 mode;
    I32 foo;
    bool bar;
} gSvBacking;
#pragma pack(pop)

// Test state vector elements.
static Element<U8> gElemMode(gSvBacking.mode);
static Element<I32> gElemFoo(gSvBacking.foo);
static Element<bool> gElemBar(gSvBacking.bar);

// Test state vector element configs.
static StateVector::ElementConfig gElemConfigs[] =
{
    {"mode", &gElemMode},
    {"foo", &gElemFoo},
    {"bar", &gElemBar},
    {}
};

// Test state vector config.
static StateVector::Config gSvConfig = {gElemConfigs, nullptr};

// State vector config with no elements.
static StateVector::ElementConfig gEmptyElemConfigs[] = {{}};
static StateVector::Config gEmptySvConfig = {gEmptyElemConfigs, nullptr};

////////////////////////////////// Test Task ///////////////////////////////////

///
/// @brief Test task that require 2 state vector elements: `I32 foo` and `bool
/// bar`. The task returns an error if stepped when bar is false. When
/// successfully stepped in safe mode, foo is decremented. When successfully
/// stepped in enable mode, foo is incremented.
///
class TestTask final : public ITask
{
public:

    TestTask(StateVector& kSv, const Element<U8>* kElemMode) :
        ITask(kElemMode), mSv(kSv), mFoo(nullptr), mBar(nullptr)
    {
    }

private:

    StateVector& mSv;

    Element<I32>* mFoo;

    Element<bool>* mBar;

    Result initImpl() final override
    {
        const Result res = mSv.getElement("foo", mFoo);
        if (res != SUCCESS)
        {
            return res;
        }
        return mSv.getElement("bar", mBar);
    }

    Result stepSafe() final override
    {
        if (!mBar->read())
        {
            return -1;
        }

        mFoo->write(mFoo->read() - 1);
        return SUCCESS;
    }

    Result stepEnable() final override
    {
        if (!mBar->read())
        {
            return -2;
        }

        mFoo->write(mFoo->read() + 1);
        return SUCCESS;
    }
};

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for the ITask interface.
///
TEST_GROUP(Task)
{
    void teardown()
    {
        // Zero state vector.
        std::memset(&gSvBacking, 0, sizeof(gSvBacking));
    }
};

///
/// @brief Stepping a task before initializing it returns an error.
///
TEST(Task, Uninitialized)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);

    // Stepping uninitialized task fails. `foo` element is unchanged.
    CHECK_ERROR(E_TSK_UNINIT, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

///
/// @brief Errors returned by ITask::initImpl() are surfaced by ITask::init().
///
TEST(Task, InitializeFail)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gEmptySvConfig, sv));
    TestTask task(sv, nullptr);

    // Task initialization fails. Task cannot step. `foo` element is unchanged.
    CHECK_ERROR(E_SV_KEY, task.init());
    CHECK_ERROR(E_TSK_UNINIT, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

///
/// @brief Initializing a task twice returns an error.
///
TEST(Task, ErrorReinitialize)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.init());
    CHECK_ERROR(E_TSK_REINIT, task.init());
}

///
/// @brief Stepping a task with an invalid mode fails.
///
TEST(Task, ErrorInvalidMode)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.init());

    // Stepping in invalid mode fails. `foo` element is unchanged.
    gElemMode.write(3);
    CHECK_ERROR(E_TSK_MODE, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

///
/// @brief Task always steps in enable mode when no mode element is provided.
///
TEST(Task, RunEnableWhenNoModeElem)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, nullptr);
    CHECK_SUCCESS(task.init());

    // No mode element was provided, so task always steps in enable mode.
    gElemBar.write(true);
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(1, gElemFoo.read());
}

///
/// @brief Task executes correct step method based on value of mode element.
///
TEST(Task, ModeSwitching)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.init());

    // Set element `bar` to true so that task steps succeed.
    gElemBar.write(true);

    // Step in disabled mode. `foo` element is unchanged.
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(0, gElemFoo.read());

    // Step in safed mode. `foo` element is decremented.
    gElemMode.write(TaskMode::SAFE);
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(-1, gElemFoo.read());

    // Step in enable mode. `foo` element is incremented twice.
    gElemMode.write(TaskMode::ENABLE);
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(0, gElemFoo.read());
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(1, gElemFoo.read());
}

///
/// @brief Errors returned by ITask::stepSafe() are surfaced by ITask::step().
///
TEST(Task, StepSafeSurfaceError)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.init());

    // With element `bar` false, stepping in safe mode returns -1.
    gElemMode.write(TaskMode::SAFE);
    CHECK_ERROR(-1, task.step());
}

///
/// @brief Errors returned by ITask::stepEnable() are surfaced by ITask::step().
///
TEST(Task, StepEnableSurfaceError)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.init());

    // With element `bar` false, stepping in enable mode returns -2.
    gElemMode.write(TaskMode::ENABLE);
    CHECK_ERROR(-2, task.step());
}
