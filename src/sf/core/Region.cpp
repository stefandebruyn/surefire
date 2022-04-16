////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Assert.hpp"
#include "sf/core/Region.hpp"
#include "sf/core/MemOps.hpp"

Region::Region(void* const kAddr, const U32 kSizeBytes) :
    Region(kAddr, kSizeBytes, nullptr)
{
}

Region::Region(void* const kAddr, const U32 kSizeBytes, ILock* const kLock) :
    mAddr(kAddr), mSizeBytes(kSizeBytes), mLock(kLock)
{
}

Result Region::write(const void* const kBuf, const U32 kBufSizeBytes)
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    if (mLock != nullptr)
    {
        // Acquire region lock.
        const Result res = mLock->acquire();
        if (res != SUCCESS)
        {
            return res;
        }
    }

    MemOps::memcpy(mAddr, kBuf, mSizeBytes);

    if (mLock != nullptr)
    {
        // Release region lock.
        const Result res = mLock->release();
        if (res != SUCCESS)
        {
            return res;
        }
    }

    return SUCCESS;
}

Result Region::read(void* const kBuf, const U32 kBufSizeBytes) const
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    if (mLock != nullptr)
    {
        // Acquire region lock.
        const Result res = mLock->acquire();
        (void) res;
        SF_ASSERT(res == SUCCESS);
    }

    MemOps::memcpy(kBuf, mAddr, mSizeBytes);

    if (mLock != nullptr)
    {
        // Release region lock.
        const Result res = mLock->release();
        (void) res;
        SF_ASSERT(res == SUCCESS);
    }

    return SUCCESS;
}

void* Region::addr() const
{
    return mAddr;
}

U32 Region::size() const
{
    return mSizeBytes;
}
