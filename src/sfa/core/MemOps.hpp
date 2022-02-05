#ifndef SFA_MEM_OPS_HPP
#define SFA_MEM_OPS_HPP

#include "sfa/core/BasicTypes.hpp"

namespace MemOps
{
    I32 strcmp(const char* kA, const char* kB);

    void* memcpy(void* kDest, const void* const kSrc, const U32 kSizeBytes);
}

#endif
