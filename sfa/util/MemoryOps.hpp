#ifndef SFA_MEMORY_OPS_HPP
#define SFA_MEMORY_OPS_HPP

#include "sfa/BasicTypes.hpp"

namespace Sfa
{

bool stringsEqual(const char* kA, const char* kB);

void* memcpy(void* kDest, const void* const kSrc, const U32 kSizeBytes);

} // namespace

#endif
