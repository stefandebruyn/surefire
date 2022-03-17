#ifndef SF_MEM_OPS_HPP
#define SF_MEM_OPS_HPP

#include "sf/core/BasicTypes.hpp"

namespace MemOps
{
    I32 strcmp(const char* kA, const char* kB);

    void* memcpy(void* kDest, const void* const kSrc, const U32 kSizeBytes);
}

#endif
