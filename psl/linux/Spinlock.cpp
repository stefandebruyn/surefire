#include "pal/Spinlock.hpp"

/////////////////////////////////// Public /////////////////////////////////////

Result Spinlock::create(Spinlock& kLock)
{
    if (pthread_spin_init(&kLock.mLock, PTHREAD_PROCESS_PRIVATE) != 0)
    {
        return E_SLK_CREATE;
    }

    kLock.mInit = true;
    return SUCCESS;
}

Result Spinlock::acquire()
{
    if (mInit == false)
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
    if (mInit == false)
    {
        return E_SLK_UNINIT;
    }

    if (pthread_spin_unlock(&mLock) != 0)
    {
        return E_SLK_REL;
    }

    return SUCCESS;
}

/////////////////////////////////// Private ////////////////////////////////////

Spinlock::Spinlock() : mInit(false)
{
}
