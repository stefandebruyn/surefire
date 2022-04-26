#ifndef SF_SPIN_EXECUTOR_HPP
#define SF_SPIN_EXECUTOR_HPP

#include "sf/core/Executor.hpp"
#include "sf/core/Task.hpp"

class SpinExecutor final : public IExecutor
{
public:

    struct Config final
    {
        ITask** tasks;
        U64 periodNs;
    };

    SpinExecutor(const SpinExecutor::Config kConfig);

    Result execute() final override;

private:

    const SpinExecutor::Config mConfig;
};

#endif
