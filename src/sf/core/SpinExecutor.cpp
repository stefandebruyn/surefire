#include "sf/core/Assert.hpp"
#include "sf/core/SpinExecutor.hpp"
#include "sf/pal/Clock.hpp"

SpinExecutor::SpinExecutor(const SpinExecutor::Config kConfig) :
    mConfig(kConfig)
{
}

Result SpinExecutor::execute()
{
    // Check that tasks are non-null.
    if (mConfig.tasks == nullptr)
    {
        return E_EXE_NULL;
    }

    // Check that clock is not within 1 year of overflow.
    static constexpr U64 oneYearNs = (Clock::NS_IN_S * 60 * 60 * 24 * 365);
    if ((0xFFFFFFFFFFFFFFFF - Clock::nanoTime()) < oneYearNs)
    {
        return E_EXE_OVFL;
    }

    // Compute average overhead of Clock::nanoTime() over 1000 cycles. This is
    // used to calibrate the post-step spinwait.
    U64 calibTotalNs = 0;
    static constexpr U32 calibCycles = 1000;
    for (U32 i = 0; i < calibCycles; ++i)
    {
        const U64 start = Clock::nanoTime();
        const U64 end = Clock::nanoTime();
        calibTotalNs += (end - start);
    }
    const U64 clockOverheadNs = (calibTotalNs / calibCycles);

    // Initialize tasks.
    for (ITask** task = mConfig.tasks; *task != nullptr; ++task)
    {
        const Result res = (*task)->init();
        if (res != SUCCESS)
        {
            return res;
        }
    }

    // Record execution start time and compute end time of first cycle.
    const U64 epochNs = Clock::nanoTime();
    U64 cycleEndNs = (epochNs + mConfig.periodNs);

    // Task execution loop.
    while (true)
    {
        // Step all tasks once.
        for (ITask** task = mConfig.tasks; *task != nullptr; ++task)
        {
            const Result res = (*task)->step();
            if (res != SUCCESS)
            {
                return res;
            }
        }

        // Spinwait for remainder of cycle. Subtract out the clock overhead to
        // make the spinwait slightly more accurate.
        while (Clock::nanoTime() < (cycleEndNs - clockOverheadNs));

        // Bump cycle end time by period.
        cycleEndNs += mConfig.periodNs;
    }

    // Unreachable.
    SF_SAFE_ASSERT(false);
}
