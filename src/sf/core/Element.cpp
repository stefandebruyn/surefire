////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Element.hpp"

namespace Sf
{

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

} // namespace Sf
