#ifndef SF_EXECUTOR_HPP
#define SF_EXECUTOR_HPP

#include "sf/core/Result.hpp"

class IExecutor
{
public:

    virtual ~IExecutor() = default;

    virtual Result execute() = 0;
};

#endif
