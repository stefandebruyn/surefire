////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Built in Austin, Texas at the University of Texas at Austin.
/// Surefire is open-source under the Apache License 2.0 - a copy of the license
/// may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Expression.hpp"

////////////////////////////////// IExprNode ///////////////////////////////////

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

//////////////////////////////////// Limits ////////////////////////////////////

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
    return (-9223372036854775807LL - 1);
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

//////////////////////////////////// Casts /////////////////////////////////////

// The following casts are from F64 to non-F64. For integer types, a non-NaN F64
// is clamped to the integer type's numeric limits. NaN becomes 0. For bool, a
// non-NaN, nonzero F64 becomes true and zero becomes false. NaN becomes false.

template<>
I8 ExprOpFuncs::safeCast<I8, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<I8>())
    {
        return Limits::min<I8>();
    }

    if (kRhs > Limits::max<I8>())
    {
        return Limits::max<I8>();
    }

    return static_cast<F64>(kRhs);
}

template<>
I16 ExprOpFuncs::safeCast<I16, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<I16>())
    {
        return Limits::min<I16>();
    }

    if (kRhs > Limits::max<I16>())
    {
        return Limits::max<I16>();
    }

    return static_cast<F64>(kRhs);
}

template<>
I32 ExprOpFuncs::safeCast<I32, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<I32>())
    {
        return Limits::min<I32>();
    }

    if (kRhs > Limits::max<I32>())
    {
        return Limits::max<I32>();
    }

    return static_cast<F64>(kRhs);
}

template<>
I64 ExprOpFuncs::safeCast<I64, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    // Note the <= and >= in this function in constrast to the usual < and >.
    // This accounts for when precision lost in the implicit F64-to-I64
    // conversion added by the compiler causes the RHS I64 value, which is
    // numerically larger than the LHS F64 value, to become equal to the LHS.
    // This also relies on the fact that when this precision loss occurs, the
    // magnitude of the cast result always goes down, not up.

    if (kRhs <= Limits::min<I64>())
    {
        return Limits::min<I64>();
    }

    if (kRhs >= Limits::max<I64>())
    {
        return Limits::max<I64>();
    }

    return static_cast<I64>(kRhs);
}

template<>
U8 ExprOpFuncs::safeCast<U8, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<U8>())
    {
        return Limits::min<U8>();
    }

    if (kRhs > Limits::max<U8>())
    {
        return Limits::max<U8>();
    }

    return static_cast<F64>(kRhs);
}

template<>
U16 ExprOpFuncs::safeCast<U16, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<U16>())
    {
        return Limits::min<U16>();
    }

    if (kRhs > Limits::max<U16>())
    {
        return Limits::max<U16>();
    }

    return static_cast<F64>(kRhs);
}

template<>
U32 ExprOpFuncs::safeCast<U32, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<U32>())
    {
        return Limits::min<U32>();
    }

    if (kRhs > Limits::max<U32>())
    {
        return Limits::max<U32>();
    }

    return static_cast<F64>(kRhs);
}

template<>
U64 ExprOpFuncs::safeCast<U64, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0;
    }

    if (kRhs < Limits::min<U64>())
    {
        return Limits::min<U64>();
    }

    // Note the >= here in constrast to the usual >. This is the same case as
    // noted in ExprOpFuncs::safeCast<I64, F64>.
    if (kRhs >= Limits::max<U64>())
    {
        return Limits::max<U64>();
    }

    return static_cast<F64>(kRhs);
}

template<>
F32 ExprOpFuncs::safeCast<F32, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0.0f;
    }

    return static_cast<F64>(kRhs);
}

template<>
bool ExprOpFuncs::safeCast<bool, F64>(const F64 kRhs)
{
    if ((kRhs != kRhs) || (kRhs == 0.0))
    {
        return false;
    }

    return true;
}

// The following casts are from non-F64 to F64. For this a static cast is
// sufficient, since F64 can exactly represent most non-F64 values and
// approximate the rest. NaNs become 0.0.

template<>
F64 ExprOpFuncs::safeCast<F64, I8>(const I8 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, I16>(const I16 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, I32>(const I32 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, I64>(const I64 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, U8>(const U8 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, U16>(const U16 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, U32>(const U32 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, U64>(const U64 kRhs)
{
    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, F32>(const F32 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0.0;
    }

    return static_cast<F64>(kRhs);
}

template<>
F64 ExprOpFuncs::safeCast<F64, F64>(const F64 kRhs)
{
    if (kRhs != kRhs)
    {
        return 0.0;
    }

    return kRhs;
}

template<>
F64 ExprOpFuncs::safeCast<F64, bool>(const bool kRhs)
{
    return static_cast<F64>(kRhs);
}
