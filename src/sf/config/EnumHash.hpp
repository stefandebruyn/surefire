#ifndef SF_ENUM_HASH_HPP
#define SF_ENUM_HASH_HPP

#include <cstddef>

struct EnumHash final
{
    template<typename T>
    std::size_t operator()(T k) const
    {
        return static_cast<std::size_t>(k);
    }
};

#endif
