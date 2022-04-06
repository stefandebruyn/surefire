////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Region.hpp"
#include "sf/core/MemOps.hpp"

Region::Region(void* const kAddr, const U32 kSizeBytes) :
    mAddr(kAddr), mSizeBytes(kSizeBytes)
{
}

Result Region::write(const void* const kBuf, const U32 kBufSizeBytes)
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    MemOps::memcpy(mAddr, kBuf, mSizeBytes);
    return SUCCESS;
}

Result Region::read(void* const kBuf, const U32 kBufSizeBytes) const
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_RGN_SIZE;
    }

    MemOps::memcpy(kBuf, mAddr, mSizeBytes);
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
