#ifndef SFA_ENUM_HASH_HPP
#define SFA_ENUM_HASH_HPP

#include <cstddef>

struct EnumHash
{
    template<typename T>
    std::size_t operator()(T k) const
    {
        return static_cast<std::size_t>(k);
    }
};

#endif
