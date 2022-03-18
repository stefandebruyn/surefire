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

//////////////////////////////// Numeric Limits ////////////////////////////////

template<>
I8 Limits::min<I8>()
{
    return -128;
}

template<>
I8 Limits::max<I8>()
{
    return 127;
}

template<>
I16 Limits::min<I16>()
{
    return -32768;
}

template<>
I16 Limits::max<I16>()
{
    return 32767;
}

template<>
I32 Limits::min<I32>()
{
    return -2147483648;
}

template<>
I32 Limits::max<I32>()
{
    return 2147483647;
}

template<>
I64 Limits::min<I64>()
{
    return (-9223372036854775807 - 1);
}

template<>
I64 Limits::max<I64>()
{
    return 9223372036854775807LL;
}

template<>
U8 Limits::min<U8>()
{
    return 0;
}

template<>
U8 Limits::max<U8>()
{
    return 255;
}

template<>
U16 Limits::min<U16>()
{
    return 0;
}

template<>
U16 Limits::max<U16>()
{
    return 65535;
}

template<>
U32 Limits::min<U32>()
{
    return 0;
}

template<>
U32 Limits::max<U32>()
{
    return 4294967295U;
}

template<>
U64 Limits::min<U64>()
{
    return 0;
}

template<>
U64 Limits::max<U64>()
{
    return 18446744073709551615ULL;
}
