#include "sfa/statevec/StateVector.hpp"
#include "sfa/UTest.hpp"

// Backing storage for test state vector.
static struct
{
    I8 i8;
    I16 i16;
    I32 i32;
    I64 i64;
    U8 u8;
    U16 u16;
    U32 u32;
    U64 u64;
    F32 f32;
    F64 f64;
    bool b;
} gBacking;

// Test state vector elements.
static Element<I8> gElemI8(gBacking.i8);
static Element<I16> gElemI16(gBacking.i16);
static Element<I32> gElemI32(gBacking.i32);
static Element<I64> gElemI64(gBacking.i64);
static Element<U8> gElemU8(gBacking.u8);
static Element<U16> gElemU16(gBacking.u16);
static Element<U32> gElemU32(gBacking.u32);
static Element<U64> gElemU64(gBacking.u64);
static Element<F32> gElemF32(gBacking.f32);
static Element<F64> gElemF64(gBacking.f64);
static Element<bool> gElemBool(gBacking.b);

// Test state vector element info.
static StateVector::ElementInfo gElems[12] =
{
    {"i8", &gElemI8},
    {"i16", &gElemI16},
    {"i32", &gElemI32},
    {"i64", &gElemI64},
    {"u8", &gElemU8},
    {"u16", &gElemU16},
    {"u32", &gElemU32},
    {"u64", &gElemU64},
    {"f32", &gElemF32},
    {"f64", &gElemF64},
    {"b", &gElemBool},
    {nullptr, nullptr}
};

// Test state vector config.
static StateVector::Config gConfig =
{
    gElems
};

TEST_GROUP(StateVector)
{
};

// Getting and writing an I8 element.
TEST(StateVector, I8Element)
{
    StateVector sv(gConfig);
    Element<I8> i8;
    CHECK_SUCCESS(sv.getElement("i8", i8));
    i8.write(1);
    CHECK_EQUAL(1, gElemI8.read());
}

// Getting and writing an I16 element.
TEST(StateVector, I16Element)
{
    StateVector sv(gConfig);
    Element<I16> i16;
    CHECK_SUCCESS(sv.getElement("i16", i16));
    i16.write(1);
    CHECK_EQUAL(1, gElemI16.read());
}

// Getting and writing an I32 element.
TEST(StateVector, I32Element)
{
    StateVector sv(gConfig);
    Element<I32> i32;
    CHECK_SUCCESS(sv.getElement("i32", i32));
    i32.write(1);
    CHECK_EQUAL(1, gElemI32.read());
}

// Getting and writing an I64 element.
TEST(StateVector, I64Element)
{
    StateVector sv(gConfig);
    Element<I64> i64;
    CHECK_SUCCESS(sv.getElement("i64", i64));
    i64.write(1);
    CHECK_EQUAL(1, (long int) gElemI64.read());
}

// Getting and writing a U8 element.
TEST(StateVector, U8Element)
{
    StateVector sv(gConfig);
    Element<U8> u8;
    CHECK_SUCCESS(sv.getElement("u8", u8));
    u8.write(1);
    CHECK_EQUAL(1, gElemU8.read());
}

// Getting and writing a U16 element.
TEST(StateVector, U16Element)
{
    StateVector sv(gConfig);
    Element<U16> u16;
    CHECK_SUCCESS(sv.getElement("u16", u16));
    u16.write(1);
    CHECK_EQUAL(1, gElemU16.read());
}

// Getting and writing a U32 element.
TEST(StateVector, U32Element)
{
    StateVector sv(gConfig);
    Element<U32> u32;
    CHECK_SUCCESS(sv.getElement("u32", u32));
    u32.write(1);
    CHECK_EQUAL(1, gElemU32.read());
}

// Getting and writing a U64 element.
TEST(StateVector, U64Element)
{
    StateVector sv(gConfig);
    Element<U64> u64;
    CHECK_SUCCESS(sv.getElement("u64", u64));
    u64.write(1);
    CHECK_EQUAL(1, (unsigned long int) gElemU64.read());
}

// Getting and writing an F32 element.
TEST(StateVector, F32Element)
{
    StateVector sv(gConfig);
    Element<F32> f32;
    CHECK_SUCCESS(sv.getElement("f32", f32));
    f32.write(1.0f);
    CHECK_EQUAL(1.0f, gElemF32.read());
}

// Getting and writing an F64 element.
TEST(StateVector, F64Element)
{
    StateVector sv(gConfig);
    Element<F64> f64;
    CHECK_SUCCESS(sv.getElement("f64", f64));
    f64.write(1.0);
    CHECK_EQUAL(1.0, gElemF64.read());
}

// Getting and writing a bool element.
TEST(StateVector, BoolElement)
{
    StateVector sv(gConfig);
    Element<bool> b;
    CHECK_SUCCESS(sv.getElement("b", b));
    b.write(true);
    CHECK_EQUAL(true, b.read());
}
