#include "sfa/sv/Element.hpp"

template<>
ElementType Element<I8>::getElementType() const
{
    return ElementType::INT8;
}

template<>
ElementType Element<I16>::getElementType() const
{
    return ElementType::INT16;
}

template<>
ElementType Element<I32>::getElementType() const
{
    return ElementType::INT32;
}

template<>
ElementType Element<I64>::getElementType() const
{
    return ElementType::INT64;
}

template<>
ElementType Element<U8>::getElementType() const
{
    return ElementType::UINT8;
}

template<>
ElementType Element<U16>::getElementType() const
{
    return ElementType::UINT16;
}

template<>
ElementType Element<U32>::getElementType() const
{
    return ElementType::UINT32;
}

template<>
ElementType Element<U64>::getElementType() const
{
    return ElementType::UINT64;
}

template<>
ElementType Element<F32>::getElementType() const
{
    return ElementType::FLOAT32;
}

template<>
ElementType Element<F64>::getElementType() const
{
    return ElementType::FLOAT64;
}

template<>
ElementType Element<bool>::getElementType() const
{
    return ElementType::BOOL;
}
