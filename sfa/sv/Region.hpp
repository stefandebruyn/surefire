#ifndef SFA_REGION_HPP
#define SFA_REGION_HPP

#include "sfa/BasicTypes.hpp"
#include "sfa/Result.hpp"

class Region final
{
public:

    constexpr Region(void* const kAddr, const U32 kSizeBytes) :
        mAddr(kAddr), mSizeBytes(kSizeBytes)
    {
    }

    Region(const Region&) = delete;
    Region(Region&&) = delete;
    Region& operator=(const Region&) = delete;
    Region& operator=(Region&&) = delete;

    Result write(const void* const kBuf, const U32 kBufSizeBytes);

    Result read(void* const kBuf, const U32 mBufSizeBytes) const;

private:

    void* const mAddr;

    const U32 mSizeBytes;
};

#endif
