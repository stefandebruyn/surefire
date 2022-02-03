#ifndef SFA_SPINLOCK_HPP
#define SFA_SPINLOCK_HPP

#include "pal/Platform.hpp"
#include "sfa/Result.hpp"

#ifdef SFA_PLATFORM_LINUX
#    include <pthread.h>
#endif

class Spinlock final
{
public:

    static Result create(Spinlock& kLock);

    Spinlock();

    Result acquire();

    Result release();

    Spinlock(const Spinlock&) = delete;
    Spinlock(Spinlock&&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;
    Spinlock& operator=(Spinlock&&) = delete;

private:

    bool mInit;

#ifdef SFA_PLATFORM_LINUX
    pthread_spinlock_t mLock;
#endif
};

#endif
