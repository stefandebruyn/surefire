#include "sfa/util/MemoryOps.hpp"

namespace Sfa
{

bool stringsEqual(const char* kA, const char* kB)
{
    bool ret = true;

    // If either string is null, that's an error. This means that two null
    // strings are not equal.
    if ((kA == nullptr) || (kB == nullptr))
    {
        ret = false;
    }
    else
    {
        while ((*kA != '\0') && (*kB != '\0'))
        {
            if (*kA != *kB)
            {
                ret = false;
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

    for (U32 i = 0; i < kSizeBytes; ++i)
    {
        *(dest + i) = *(src + i);
    }

    return dest;
}

} // namespace Sfa
