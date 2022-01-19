#include "sfa/util/MemOps.hpp"

I32 MemOps::strcmp(const char* kA, const char* kB)
{
    I32 ret = 0;

    if ((kA != nullptr) != (kB != nullptr))
    {
        if (kA == nullptr)
        {
            ret = kB[0];
        }
        else
        {
            ret = kA[0];
        }
    }
    else if ((kA != nullptr) && (kB != nullptr))
    {
        do
        {
            ret = (*kA - *kB);
            if (ret != 0)
            {
                break;
            }
        }
        while ((*kA++ != '\0') && (*kB++ != '\0'));
    }

    return ret;
}

void* MemOps::memcpy(void* kDest, const void* const kSrc, const U32 kSizeBytes)
{
    char* const dest = static_cast<char* const>(kDest);
    const char* const src = static_cast<const char* const>(kSrc);

    if ((dest != nullptr) && (src != nullptr))
    {
        for (U32 i = 0; i < kSizeBytes; ++i)
        {
            *(dest + i) = *(src + i);
        }
    }

    return dest;
}
