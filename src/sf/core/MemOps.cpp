////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/MemOps.hpp"

I32 MemOps::strcmp(const char* kA, const char* kB)
{
    I32 ret = 0;

    if ((kA != nullptr) != (kB != nullptr))
    {
        // One string is null, but not both - treat the null string like an
        // empty string.
        if (kA == nullptr)
        {
            ret = -kB[0];
        }
        else
        {
            ret = kA[0];
        }
    }
    else if ((kA != nullptr) && (kB != nullptr))
    {
        // Both strings are non-null - scan through for the first non-matching
        // character and return their difference.
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

void* MemOps::memcpy(void* kDest, const void* const kSrc, const U32 kBytes)
{
    char* const dest = static_cast<char*>(kDest);
    const char* const src = static_cast<const char*>(kSrc);

    // TODO: This could be made more efficient by moving >1 byte at once...
    if ((dest != nullptr) && (src != nullptr))
    {
        for (U32 i = 0; i < kBytes; ++i)
        {
            *(dest + i) = *(src + i);
        }
    }

    return dest;
}
