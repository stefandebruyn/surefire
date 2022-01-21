#include <cstring>

#include "sfa/task/ITask.hpp"
#include "UTest.hpp"

/***************************** STATE VECTOR CONFIG ****************************/

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

/********************************* TEST TASK **********************************/

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
        if (mBar->read() == false)
        {
            return E_STATE;
        }

        mFoo->write(mFoo->read() - 1);
        return SUCCESS;
    }

    Result stepEnable() final override
    {
        if (mBar->read() == false)
        {
            return E_ENUM;
        }

        mFoo->write(mFoo->read() + 1);
        return SUCCESS;
    }
};

/*********************************** TESTS ************************************/

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
    CHECK_EQUAL(E_UNINITIALIZED, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

TEST(ITask, InitializeFail)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gEmptySvConfig, sv));
    TestTask task(sv, nullptr);

    // Task initialization fails. Task cannot step. `foo` element is unchanged.
    CHECK_EQUAL(E_KEY, task.initialize());
    CHECK_EQUAL(E_UNINITIALIZED, task.step());
    CHECK_EQUAL(0, gElemFoo.read());
}

TEST(ITask, InvalidMode)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

    // Stepping in invalid mode fails. `foo` element is unchanged.
    gElemMode.write(3);
    CHECK_EQUAL(E_ENUM, task.step());
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
    gElemMode.write(MODE_SAFE);
    CHECK_SUCCESS(task.step());
    CHECK_EQUAL(-1, gElemFoo.read());

    // Step in enable mode. `foo` element is incremented twice.
    gElemMode.write(MODE_ENABLE);
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

    // With element `bar` false, stepping in safe mode returns `E_STATE`.
    gElemMode.write(MODE_SAFE);
    CHECK_EQUAL(E_STATE, task.step());
}

TEST(ITask, StepEnableSurfaceError)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gSvConfig, sv));
    TestTask task(sv, &gElemMode);
    CHECK_SUCCESS(task.initialize());

    // With element `bar` false, stepping in enabled mode returns `E_ENUM`.
    gElemMode.write(MODE_ENABLE);
    CHECK_EQUAL(E_ENUM, task.step());
}