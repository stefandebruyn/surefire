#include "sfa/sv/StateVector.hpp"
#include "utest/UTest.hpp"

// Backing storage for test state vector.
#pragma pack(push, 1)
static struct
{
    struct
    {
        I8 i8;
        I16 i16;
        I32 i32;
        I64 i64;
    } foo;
    struct
    {
        U8 u8;
        U16 u16;
        U32 u32;
        U64 u64;
        F32 f32;
        F64 f64;
        bool b;
    } bar;
} gBacking;
#pragma pack(pop)

// Test state vector elements.
static Element<I8> gElemI8(gBacking.foo.i8);
static Element<I16> gElemI16(gBacking.foo.i16);
static Element<I32> gElemI32(gBacking.foo.i32);
static Element<I64> gElemI64(gBacking.foo.i64);
static Element<U8> gElemU8(gBacking.bar.u8);
static Element<U16> gElemU16(gBacking.bar.u16);
static Element<U32> gElemU32(gBacking.bar.u32);
static Element<U64> gElemU64(gBacking.bar.u64);
static Element<F32> gElemF32(gBacking.bar.f32);
static Element<F64> gElemF64(gBacking.bar.f64);
static Element<bool> gElemBool(gBacking.bar.b);

// Test state vector element configs.
static StateVector::ElementConfig gElems[12] =
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

// Test state vector regions.
static Region gRegionFoo(&gBacking.foo, sizeof(gBacking.foo));
static Region gRegionBar(&gBacking.bar, sizeof(gBacking.bar));

// Test state vector region configs.
static StateVector::RegionConfig gRegions[3] =
{
    {"foo", &gRegionFoo},
    {"bar", &gRegionBar},
    {nullptr, nullptr}
};

// Test state vector config.
static StateVector::Config gConfig = {gElems, gRegions};

TEST_GROUP(StateVector)
{
};

// Getting I8 element.
TEST(StateVector, I8Element)
{
    StateVector sv(gConfig);
    Element<I8>* i8 = nullptr;
    CHECK_SUCCESS(sv.getElement("i8", i8));
    POINTERS_EQUAL(i8, &gElemI8);
}

// Getting regions.
TEST(StateVector, GetRegion)
{
    StateVector sv(gConfig);
    Region* region = nullptr;
    CHECK_SUCCESS(sv.getRegion("foo", region));
    POINTERS_EQUAL(region, &gRegionFoo);
    CHECK_SUCCESS(sv.getRegion("bar", region));
    POINTERS_EQUAL(region, &gRegionBar);
    CHECK_EQUAL(E_KEY, sv.getRegion("baz", region));
}
