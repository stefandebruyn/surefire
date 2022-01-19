#include "sfa/sv/Element.hpp"
#include "sfa/BasicTypes.hpp"
#include "UTest.hpp"

/////////////////////////////////// Helpers ////////////////////////////////////

template<typename T>
static void testGetType(const ElementType kType)
{
    T backing = 0;
    Element<T> elem(backing);
    CHECK_EQUAL(kType, elem.type());
}

template<typename T>
static void testReadWrite(const T kInitVal, const T kWriteVal)
{
    T backing = kInitVal;
    Element<T> elem(backing);
    CHECK_EQUAL(kInitVal, elem.read());
    elem.write(kWriteVal);
    CHECK_EQUAL(kWriteVal, elem.read());
    CHECK_EQUAL(kWriteVal, backing);
}

template<typename T>
static void testGetAddr()
{
    T backing = 0;
    Element<T> elem(backing);
    POINTERS_EQUAL(&backing, elem.addr());
}

template<typename T>
static void testGetSize()
{
    T backing = 0;
    Element<T> elem(backing);
    CHECK_EQUAL(sizeof(T), elem.size());
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(Element)
{
};

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

TEST(Element, GetType)
{
    testGetType<I8>(INT8);
    testGetType<I16>(INT16);
    testGetType<I32>(INT32);
    testGetType<I64>(INT64);
    testGetType<U8>(UINT8);
    testGetType<U16>(UINT16);
    testGetType<U32>(UINT32);
    testGetType<U64>(UINT64);
    testGetType<F32>(FLOAT32);
    testGetType<F64>(FLOAT64);
    testGetType<bool>(BOOL);
}

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
