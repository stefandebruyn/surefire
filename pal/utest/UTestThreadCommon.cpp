#include "UTestThreadCommon.hpp"

Result nop(void* kArgs)
{
    return SUCCESS;
}

Result setFlag(void* kArgs)
{
    *(static_cast<bool*>(kArgs)) = true;
    return SUCCESS;
}
