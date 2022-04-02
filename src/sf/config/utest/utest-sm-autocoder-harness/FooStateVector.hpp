///
/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.
///

#ifndef FooStateVector_HPP
#define FooStateVector_HPP

#include "sf/core/StateVector.hpp"

namespace FooStateVector
{

///
/// @brief Initializes a state vector from the autocoded config.
///
/// @note The config is static. This function should only be called once.
///
/// @param[out] kSv  State vector to initialize.
///
/// @retval SUCCESS  Successfully initialized state vector.
/// @retval [other]  Initialization failed.
///
static Result getConfig(StateVector::Config& kSvConfig)
{
    #pragma pack(push, 1)
    static struct
    {
        struct
        {
            U32 state;
            U64 time;
            U64 n;
            U64 fib_n;
        } Foo;
    } backing;
    #pragma pack(pop)

    static Element<U32> elemstate(backing.Foo.state);
    static Element<U64> elemtime(backing.Foo.time);
    static Element<U64> elemn(backing.Foo.n);
    static Element<U64> elemfib_n(backing.Foo.fib_n);

    static Region regionFoo(&backing.Foo, sizeof(backing.Foo));

    static StateVector::ElementConfig elemConfigs[] =
    {
        {"state", &elemstate},
        {"time", &elemtime},
        {"n", &elemn},
        {"fib_n", &elemfib_n},
        {nullptr, nullptr}
    };

    static StateVector::RegionConfig regionConfigs[] =
    {
        {"Foo", &regionFoo},
        {nullptr, nullptr}
    };

    kSvConfig = {elemConfigs, regionConfigs};

    return SUCCESS;
}

} // namespace FooStateVector

#endif
