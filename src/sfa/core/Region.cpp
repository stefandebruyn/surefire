#include "sfa/core/Region.hpp"
#include "sfa/core/MemOps.hpp"

Result Region::write(const void* const kBuf, const U32 kBufSizeBytes)
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_SIZE;
    }

    MemOps::memcpy(mAddr, kBuf, mSizeBytes);
    return SUCCESS;
}

Result Region::read(void* const kBuf, const U32 kBufSizeBytes) const
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_SIZE;
    }

    MemOps::memcpy(kBuf, mAddr, mSizeBytes);
    return SUCCESS;
}

const void* Region::addr() const
{
    return mAddr;
}

U32 Region::size() const
{
    return mSizeBytes;
}
