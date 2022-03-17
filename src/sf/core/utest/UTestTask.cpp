#include <cstring>

#include "sf/core/Task.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

#pragma pack(push, 1)
static struct
{
    U8 mode;
    I32 foo;
    bool bar;
} gSvBacking;
#pragma pack(pop)

static Element<U8> gElemMode(gSvBacking.mode);
static Element<I32> gElemFoo(gSvBacking.foo);
static Element<bool> gElemBar(gSvBacking.bar);

static StateVector::ElementConfig gElemConfigs[] =
{
    {"mode", &gElemMode},
    {"foo", &gElemFoo},
    {"bar", &gElemBar},
    {}
};

static StateVector::Config gSvConfig = {gElemConfigs, nullptr};

static StateVector::ElementConfig gEmptyElemConfigs[] = {{}};
static StateVector::Config gEmptySvConfig = {gEmptyElemConfigs, nullptr};

////////////////////////////////// Test Task ///////////////////////////////////

class TestTask final : public ITask
{
public:

    constexpr TestTask(StateVector& kSv, const Element<U8>* kModeElem) :
        ITask(kSv, kModeElem), mFoo(nullptr), mBar(nullptr)
    {
    }

private:

    Element<I32>* mFoo;

    Element<bool>* mBar;

    Result initializeImpl() final override
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

TEST_GROUP(ITask)
{
    void teardown()
    {
        // Zero state vector.
        std::memset(&gSvBacking, 0, sizeof(gSvBacking));
    }
};

TEST(ITask, Uninitialized)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);

    // Stepping uninitialized task fails. `foo` element is unchanged.
    CHECK_ERROR(E_TSK_UNINIT, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

TEST(ITask, InitializeFail)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gEmptySvConfig, sv));
    TestTask task(sv, nullptr);

    // Task initialization fails. Task cannot step. `foo` element is unchanged.
    CHECK_ERROR(E_SV_KEY, task.initialize());
    CHECK_ERROR(E_TSK_UNINIT, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

TEST(ITask, ErrorReinitialize)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());
    CHECK_ERROR(E_TSK_REINIT, task.initialize());
}

TEST(ITask, InvalidMode)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

    // Stepping in invalid mode fails. `foo` element is unchanged.
    gElemMode.write(3);
    CHECK_ERROR(E_TSK_MODE, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

TEST(ITask, RunEnableWhenNoModeElem)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, nullptr);
    CHECK_SUCCESS(task.initialize());

    // No mode element was provided, so task always steps in enabled mode.
    gElemBar.write(true);
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(1, gElemFoo.read());
}

TEST(ITask, ModeSwitching)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

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

TEST(ITask, StepSafeSurfaceError)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

    // With element `bar` false, stepping in safe mode returns -1.
    gElemMode.write(TaskMode::SAFE);
    CHECK_ERROR(-1, task.step());
}

TEST(ITask, StepEnableSurfaceError)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

    // With element `bar` false, stepping in enabled mode returns -2.
    gElemMode.write(TaskMode::ENABLE);
    CHECK_ERROR(-2, task.step());
}
