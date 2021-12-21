#include "sfa/sv/Region.hpp"
#include "sfa/util/MemoryOps.hpp"

Result Region::write(const void* const kBuf, const U32 kBufSizeBytes)
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_SIZE;
    }

    Sfa::memcpy(mAddr, kBuf, mSizeBytes);
    return SUCCESS;
}

Result Region::read(void* const kBuf, const U32 kBufSizeBytes) const
{
    if (kBufSizeBytes != mSizeBytes)
    {
        return E_SIZE;
    }

    Sfa::memcpy(kBuf, mAddr, mSizeBytes);
    return SUCCESS;
}

const void* Region::getAddr() const
{
    return mAddr;
}

U32 Region::getSizeBytes() const
{
    return mSizeBytes;
}
