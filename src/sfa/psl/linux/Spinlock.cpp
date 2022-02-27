#include "sfa/pal/Spinlock.hpp"

Result Spinlock::create(Spinlock& kLock)
{
    if (pthread_spin_init(&kLock.mLock, PTHREAD_PROCESS_PRIVATE) != 0)
    {
        return E_SLK_CREATE;
    }

    kLock.mInit = true;
    return SUCCESS;
}

Spinlock::Spinlock() : mInit(false)
{
}

Spinlock::~Spinlock()
{
    (void) pthread_spin_destroy(&mLock);
}

Result Spinlock::acquire()
{
    if (!mInit)
    {
        return E_SLK_UNINIT;
    }

    if (pthread_spin_lock(&mLock) != 0)
    {
        return E_SLK_ACQ;
    }

    return SUCCESS;
}

Result Spinlock::release()
{
    if (!mInit)
    {
        return E_SLK_UNINIT;
    }

    if (pthread_spin_unlock(&mLock) != 0)
    {
        return E_SLK_REL;
    }

    return SUCCESS;
}
