#include "sf/core/Expression.hpp"

template<>
ElementType IExprNode<I8>::type() const
{
    return ElementType::INT8;
}

template<>
ElementType IExprNode<I16>::type() const
{
    return ElementType::INT16;
}

template<>
ElementType IExprNode<I32>::type() const
{
    return ElementType::INT32;
}

template<>
ElementType IExprNode<I64>::type() const
{
    return ElementType::INT64;
}

template<>
ElementType IExprNode<U8>::type() const
{
    return ElementType::UINT8;
}

template<>
ElementType IExprNode<U16>::type() const
{
    return ElementType::UINT16;
}

template<>
ElementType IExprNode<U32>::type() const
{
    return ElementType::UINT32;
}

template<>
ElementType IExprNode<U64>::type() const
{
    return ElementType::UINT64;
}

template<>
ElementType IExprNode<F32>::type() const
{
    return ElementType::FLOAT32;
}

template<>
ElementType IExprNode<F64>::type() const
{
    return ElementType::FLOAT64;
}

template<>
ElementType IExprNode<bool>::type() const
{
    return ElementType::BOOL;
}
