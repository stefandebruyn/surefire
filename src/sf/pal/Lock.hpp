#ifndef SF_LOCK_HPP
#define SF_LOCK_HPP

#include "sf/core/Result.hpp"

class ILock
{
public:

    virtual Result acquire() = 0;

    virtual Result release() = 0;
};

#endif
