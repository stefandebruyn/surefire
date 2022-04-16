#ifndef SF_SPINLOCK_HPP
#define SF_SPINLOCK_HPP

#ifdef SF_PLATFORM_LINUX
#    include <pthread.h>
#endif

#include "sf/core/Result.hpp"
#include "sf/pal/Lock.hpp"

class Spinlock final : public ILock
{
public:

    static Result init(Spinlock& kLock);

    Spinlock();

    ~Spinlock();

    Result acquire() final override;

    Result release() final override;

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
