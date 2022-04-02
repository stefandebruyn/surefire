#ifndef SF_SPINLOCK_HPP
#define SF_SPINLOCK_HPP

#ifdef SF_PLATFORM_LINUX
#    include <pthread.h>
#endif

#include "sf/core/Result.hpp"

class Spinlock final
{
public:

    static Result init(Spinlock& kLock);

    Spinlock();

    ~Spinlock();

    Result acquire();

    Result release();

    Spinlock(const Spinlock&) = delete;
    Spinlock(Spinlock&&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;
    Spinlock& operator=(Spinlock&&) = delete;

private:

    bool mInit;

#ifdef SF_PLATFORM_LINUX
    pthread_spinlock_t mLock;
#endif
};

#endif
