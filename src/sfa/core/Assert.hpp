#ifndef SFA_ASSERT_HPP
#define SFA_ASSERT_HPP

#include "sfa/pal/Console.hpp"
#include "sfa/pal/System.hpp"

#ifdef SFA_ENABLE_ASSERTS
#    define SFA_ASSERT(kExpr)                                                  \
do                                                                             \
{                                                                              \
    const bool _res = (kExpr);                                                 \
    if (_res != true)                                                          \
    {                                                                          \
        Console::printf("\nAssertion failed at %s:%d:\n    %s\n",              \
                        __FILE__, __LINE__, #kExpr);                           \
        System::exit(1);                                                       \
    }                                                                          \
} while (false);
#else
#    define SFA_ASSERT(kExpr)
#endif

#endif
