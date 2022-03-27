#include "sf/core/Element.hpp"

template<>
ElementType Element<I8>::type() const
{
    return ElementType::INT8;
}

template<>
ElementType Element<I16>::type() const
{
    return ElementType::INT16;
}

template<>
ElementType Element<I32>::type() const
{
    return ElementType::INT32;
}

template<>
ElementType Element<I64>::type() const
{
    return ElementType::INT64;
}

template<>
ElementType Element<U8>::type() const
{
    return ElementType::UINT8;
}

template<>
ElementType Element<U16>::type() const
{
    return ElementType::UINT16;
}

template<>
ElementType Element<U32>::type() const
{
    return ElementType::UINT32;
}

template<>
ElementType Element<U64>::type() const
{
    return ElementType::UINT64;
}

template<>
ElementType Element<F32>::type() const
{
    return ElementType::FLOAT32;
}

template<>
ElementType Element<F64>::type() const
{
    return ElementType::FLOAT64;
}

template<>
ElementType Element<bool>::type() const
{
    return ElementType::BOOL;
}
