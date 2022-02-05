#ifndef SFA_REGION_HPP
#define SFA_REGION_HPP

#include "sfa/core/BasicTypes.hpp"
#include "sfa/core/Result.hpp"

class Region final
{
public:

    constexpr Region(void* const kAddr, const U32 kSizeBytes) :
        mAddr(kAddr), mSizeBytes(kSizeBytes)
    {
    }

    Result write(const void* const kBuf, const U32 kBufSizeBytes);

    Result read(void* const kBuf, const U32 mBufSizeBytes) const;

    const void* addr() const;

    U32 size() const;

    Region(const Region&) = delete;
    Region(Region&&) = delete;
    Region& operator=(const Region&) = delete;
    Region& operator=(Region&&) = delete;

private:

    void* const mAddr;

    const U32 mSizeBytes;
};

#endif
