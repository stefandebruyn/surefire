#ifndef SF_ASSERT_HPP
#define SF_ASSERT_HPP

#include "sf/pal/Console.hpp"
#include "sf/pal/System.hpp"

#ifdef SF_ENABLE_ASSERTS
#    define SF_ASSERT(kExpr)                                                   \
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
#    define SF_ASSERT(kExpr)
#endif

#endif
