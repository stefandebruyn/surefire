#ifndef SFA_DIAG_HPP
#define SFA_DIAG_HPP

#include "sfa/core/Result.hpp"

namespace Diag
{
    void printOnError(const Result kRes, const char* const kMsg);

    void haltOnError(const Result kRes, const char* const kMsg);
}

#endif
