#include "sfa/util/MemOps.hpp"

namespace MemOps
{

I32 strcmp(const char* kA, const char* kB)
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
        while ((*kA != '\0') && (*kB != '\0'))
        {
            if (*kA != *kB)
            {
                ret = (*kA - *kB);
                break;
            }
            ++kA;
            ++kB;
        }
    }

    return ret;
}

void* memcpy(void* kDest, const void* const kSrc, const U32 kSizeBytes)
{
    char* const dest = (char* const) kDest;
    const char* const src = (const char* const) kSrc;

    if ((dest != nullptr) && (src != nullptr))
    {
        for (U32 i = 0; i < kSizeBytes; ++i)
        {
            *(dest + i) = *(src + i);
        }
    }

    return dest;
}

} // namespace Sfa
