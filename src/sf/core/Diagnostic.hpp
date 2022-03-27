#ifndef SF_DIAG_HPP
#define SF_DIAG_HPP

#include "sf/core/Result.hpp"

namespace Diag
{
    void printOnError(const Result kRes, const char* const kMsg);

    void haltOnError(const Result kRes, const char* const kMsg);

    inline void errsc(const Result kRes, Result& kStorage)
    {
        if ((kRes != SUCCESS) && (kStorage == SUCCESS))
        {
            kStorage = kRes;
        }
    }
}

#endif
