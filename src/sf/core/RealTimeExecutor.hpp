#ifndef SF_REAL_TIME_EXECUTOR_HPP
#define SF_REAL_TIME_EXECUTOR_HPP

#include "sf/core/Executor.hpp"
#include "sf/core/Task.hpp"
#include "sf/pal/Clock.hpp"
#include "sf/pal/Thread.hpp"

template<U32 TCores>
class RealTimeExecutor final : public IExecutor
{
public:

    struct CoreConfig final
    {
        ITask** tasks;
        U8 coreNum;
        U64 periodNs;
    };

    struct Config final
    {
        RealTimeExecutor::CoreConfig cores[TCores];
        I32 priority;
    };

    RealTimeExecutor(const RealTimeExecutor::Config kConfig) : mConfig(kConfig)
    {
    }

    Result execute() final override
    {
        // Validate executor config.
        Result res = this->validateConfig();
        if (res != SUCCESS)
        {
            return res;
        }

        // Check that clock is not within 1 year of overflow.
        static constexpr U64 oneYearNs = (Clock::NS_IN_S * 60 * 60 * 24 * 365);
        if ((0xFFFFFFFFFFFFFFFF - Clock::nanoTime()) < oneYearNs)
        {
            return E_EXE_OVFL;
        }

        // Compute average overhead of Clock::nanoTime() over 1000 cycles. This
        // is used to calibrate the task thread post-step spinwait.
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
        for (U32 i = 0; i < TCores; ++i)
        {
            for (ITask** task = mConfig.cores[i].tasks;
                 *task != nullptr;
                 ++task)
            {
                const Result res = (*task)->init();
                if (res != SUCCESS)
                {
                    return res;
                }
            }
        }

        // Set the current thread priority to 1 higher than the highest priority
        // core config to make sure we don't get preempted mid-executor startup.
        res = Thread::set((mConfig.priority + 1),
                          Thread::REALTIME,
                          Thread::currentCore());
        if (res != SUCCESS)
        {
            return res;
        }

        // Create task threads on configured cores.
        for (U32 i = 0; i < TCores; ++i)
        {
            // Set up thread args.
            mThreadArgs[i] =
            {
                mConfig.cores[i].tasks,
                mConfig.cores[i].periodNs,
                clockOverheadNs
            };

            // Create thread.
            res = Thread::init(RealTimeExecutor::threadWrapper,
                               static_cast<void*>(&mThreadArgs[i]),
                               mConfig.priority,
                               Thread::REALTIME,
                               mConfig.cores[i].coreNum,
                               mThreads[i]);
            if (res != SUCCESS)
            {
                return res;
            }
        }

        // Wait for first thread to finish. In the nominal case, this never
        // returns.
        const Result awaitRes = mThreads[0].await(&res);
        if (awaitRes != SUCCESS)
        {
            return awaitRes;
        }

        return res;
    }

private:

    struct ThreadArgs final
    {
        ITask** tasks;
        U64 periodNs;
        U64 clockOverheadNs;
    };

    const RealTimeExecutor::Config mConfig;

    Thread mThreads[TCores];

    ThreadArgs mThreadArgs[TCores];

    static Result threadWrapper(void* kArgs)
    {
        ThreadArgs* const args = static_cast<ThreadArgs*>(kArgs);

        // Record execution start time and compute end time of first cycle.
        const U64 epochNs = Clock::nanoTime();
        U64 cycleEndNs = (epochNs + args->periodNs);

        // Task execution loop.
        while (true)
        {
            // Step all tasks once.
            for (ITask** task = args->tasks; *task != nullptr; ++task)
            {
                const Result res = (*task)->step();
                if (res != SUCCESS)
                {
                    return res;
                }
            }

            // Spinwait for remainder of cycle. Subtract out the clock overhead
            // to make the spinwait slightly more accurate.
            SF_SAFE_ASSERT(cycleEndNs >= args->clockOverheadNs);
            while (Clock::nanoTime() < (cycleEndNs - args->clockOverheadNs));

            // Bump cycle end time by period.
            cycleEndNs += args->periodNs;
        }

        // Unreachable.
        SF_SAFE_ASSERT(false);
    }

    Result validateConfig()
    {
        // Executor must use at least 1 core.
        if (TCores == 0)
        {
            return E_MSE_CNT;
        }

        // Check that core configs are non-null.
        if (mConfig.cores == nullptr)
        {
            return E_EXE_NULL;
        }

        // Check that core configs have unique core numbers.
        for (const RealTimeExecutor::CoreConfig* core = mConfig.cores;
            core->tasks != nullptr;
            ++core)
        {
            for (const RealTimeExecutor::CoreConfig* otherCore = mConfig.cores;
                otherCore->tasks != nullptr;
                ++otherCore)
            {
                if ((core != otherCore)
                    && (core->coreNum == otherCore->coreNum))
                {
                    return E_MSE_CORE;
                }
            }
        }

        // Unreachable;
        SF_SAFE_ASSERT(false);
    }
};

#endif
