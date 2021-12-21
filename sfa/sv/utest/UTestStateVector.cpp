#include "sfa/sv/StateVector.hpp"
#include "utest/UTest.hpp"

// Test state vector backing storage.
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
static StateVector::ElementConfig gElems[] =
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
    {}
};

// Test state vector regions.
static Region gRegionFoo(&gBacking.foo, sizeof(gBacking.foo));
static Region gRegionBar(&gBacking.bar, sizeof(gBacking.bar));

// Test state vector region configs.
static StateVector::RegionConfig gRegions[] =
{
    {"foo", &gRegionFoo},
    {"bar", &gRegionBar},
    {}
};

// Test state vector config.
static StateVector::Config gConfig = {gElems, gRegions};

TEST_GROUP(StateVector)
{
    void teardown()
    {
        // `AllowElementMisalignmentWithoutRegions` can fail without restoring
        // the `i8` element pointer in `gConfig`, so do that here.
        gConfig.elems[0].elem = &gElemI8;
    }
};

// Getting I8 element.
TEST(StateVector, I8Element)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gConfig, sv));
    Element<I8>* i8 = nullptr;
    CHECK_SUCCESS(sv.getElement("i8", i8));
    POINTERS_EQUAL(i8, &gElemI8);
}

// Getting regions.
TEST(StateVector, GetRegion)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::create(gConfig, sv));
    Region* region = nullptr;
    CHECK_SUCCESS(sv.getRegion("foo", region));
    POINTERS_EQUAL(region, &gRegionFoo);
    CHECK_SUCCESS(sv.getRegion("bar", region));
    POINTERS_EQUAL(region, &gRegionBar);
    CHECK_EQUAL(E_KEY, sv.getRegion("baz", region));
}

// Non-contiguous elements are allowed when not using regions.
TEST(StateVector, AllowElementMisalignmentWithoutRegions)
{
    // Replace `i8` element with one outside the state vector backing storage
    // and null out the regions array.
    I8 backing = 0.0;
    Element<I8> elem(backing);
    IElement* const tmp0 = gConfig.elems[0].elem;
    gConfig.elems[0].elem = &elem;
    StateVector::RegionConfig* const tmp1 = gConfig.regions;
    gConfig.regions = nullptr;

    // Creating state vector.
    StateVector sv;
    Result res = StateVector::create(gConfig, sv);
    gConfig.regions = tmp1;
    CHECK_SUCCESS(res);

    // Getting `i8` element returns the element created above.
    Element<I8>* i8 = nullptr;
    res = sv.getElement("i8", i8);
    gConfig.elems[0].elem = tmp0;
    CHECK_SUCCESS(res);
    POINTERS_EQUAL(&elem, i8);
}
