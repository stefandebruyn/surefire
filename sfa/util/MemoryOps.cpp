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

} // namespace Sfa
