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
///
///                             ---------------
/// @file  sf/core/utest/UTestElement.cpp
/// @brief Unit tests for Element.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/BasicTypes.hpp"
#include "sf/core/Element.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that Element::type() returns the expected enum.
///
/// @tparam T  Element type.
///
/// @param[in] kType  Expected type enum.
///
template<typename T>
static void testGetType(const ElementType kType)
{
    T backing = 0;
    Element<T> elem(backing);
    CHECK_EQUAL(kType, elem.type());
}

///
/// @brief Checks that an element can be read, and writing the element updates
/// its backing.
///
/// @tparam T  Element type.
///
/// @param[in] kInitVal   Element initial value.
/// @param[in] kWriteVal  Value to write to element and then read back.
///
template<typename T>
static void testReadWrite(const T kInitVal, const T kWriteVal)
{
    // Create element with initial value.
    T backing = kInitVal;
    Element<T> elem(backing);

    // Reading element returns initial value.
    CHECK_EQUAL(kInitVal, elem.read());

    // Write new value. Reading element returns the new value, and the element
    // backing was updated accordingly.
    elem.write(kWriteVal);
    CHECK_EQUAL(kWriteVal, elem.read());
    CHECK_EQUAL(kWriteVal, backing);
}

///
/// @brief Checks that Element::addr() returns the backing address.
///
/// @tparam T  Element type.
///
template<typename T>
static void testGetAddr()
{
    T backing = 0;
    Element<T> elem(backing);
    POINTERS_EQUAL(&backing, elem.addr());
}

///
/// @brief Checks that Element::size() returns the element type size.
///
/// @tparam T  Element type.
///
template<typename T>
static void testGetSize()
{
    T backing = 0;
    Element<T> elem(backing);
    CHECK_EQUAL(sizeof(T), elem.size());
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for Element.
///
TEST_GROUP(Element)
{
};

///
/// @test Elements are read and written correctly.
///
TEST(Element, ReadWrite)
{
    testReadWrite<I8>(-101, 23);
    testReadWrite<I16>(12443, -438);
    testReadWrite<I32>(1065779324, -996103);
    testReadWrite<I64>(-12566034892L, 654223);
    testReadWrite<U8>(101, 255);
    testReadWrite<U16>(3001, 8888);
    testReadWrite<U32>(21903, 3862999091U);
    testReadWrite<U64>(12, 23001040778UL);
    testReadWrite<F32>(0.000233391f, -415.131313f);
    testReadWrite<F64>(-1.522, 903.88854112);
    testReadWrite<bool>(false, true);
}

///
/// @test Element::type() returns the correct type enum.
///
TEST(Element, GetType)
{
    testGetType<I8>(ElementType::INT8);
    testGetType<I16>(ElementType::INT16);
    testGetType<I32>(ElementType::INT32);
    testGetType<I64>(ElementType::INT64);
    testGetType<U8>(ElementType::UINT8);
    testGetType<U16>(ElementType::UINT16);
    testGetType<U32>(ElementType::UINT32);
    testGetType<U64>(ElementType::UINT64);
    testGetType<F32>(ElementType::FLOAT32);
    testGetType<F64>(ElementType::FLOAT64);
    testGetType<bool>(ElementType::BOOL);
}

///
/// @test Element::addr() returns the correct backing address.
///
TEST(Element, GetAddr)
{
    testGetAddr<I8>();
    testGetAddr<I16>();
    testGetAddr<I32>();
    testGetAddr<I64>();
    testGetAddr<U8>();
    testGetAddr<U16>();
    testGetAddr<U32>();
    testGetAddr<U64>();
    testGetAddr<F32>();
    testGetAddr<F64>();
    testGetAddr<bool>();
}

///
/// @test Element::size() returns the correct type size.
///
TEST(Element, GetSize)
{
    testGetSize<I8>();
    testGetSize<I16>();
    testGetSize<I32>();
    testGetSize<I64>();
    testGetSize<U8>();
    testGetSize<U16>();
    testGetSize<U32>();
    testGetSize<U64>();
    testGetSize<F32>();
    testGetSize<F64>();
    testGetSize<bool>();
}
