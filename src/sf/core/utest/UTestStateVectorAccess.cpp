////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building avionics
/// software applications. Built in Austin, Texas at the University of Texas at
/// Austin. Surefire is open-source under the Apache License 2.0 - a copy of the
/// license may be obtained at https://www.apache.org/licenses/LICENSE-2.0.
/// Surefire is maintained at https://www.github.com/stefandebruyn/surefire.
///
///                             ---------------
/// @file  sf/core/utest/UTestStateVectorAccess.cpp
/// @brief Unit tests for accessing StateVector elements and regions.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/StateVector.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

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
    {"bool", &gElemBool},
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

/////////////////////////////////// Helpers ////////////////////////////////////

///
/// @brief Checks that StateVector::getElement() and StateVector::getIElement()
/// return the correct pointers.
///
/// @param[in] kName  Element name.
/// @param[in] kElem  Element object.
///
template<typename T>
static void testGetElement(const char* const kName, const Element<T>& kElem)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    Element<T>* elem = nullptr;
    CHECK_SUCCESS(sv.getElement(kName, elem));
    POINTERS_EQUAL(&kElem, elem);
    IElement* ielem = nullptr;
    CHECK_SUCCESS(sv.getIElement(kName, ielem));
    POINTERS_EQUAL(&kElem, ielem);
}

//////////////////////////////////// Tests /////////////////////////////////////

///
/// @brief Unit tests for accessing StateVector elements and regions.
///
TEST_GROUP(StateVectorAccess)
{
};

///
/// @brief Element lookup returns the correct pointer.
///
TEST(StateVectorAccess, GetElement)
{
    testGetElement<I8>("i8", gElemI8);
    testGetElement<I16>("i16", gElemI16);
    testGetElement<I32>("i32", gElemI32);
    testGetElement<I64>("i64", gElemI64);
    testGetElement<U8>("u8", gElemU8);
    testGetElement<U16>("u16", gElemU16);
    testGetElement<U32>("u32", gElemU32);
    testGetElement<U64>("u64", gElemU64);
    testGetElement<F32>("f32", gElemF32);
    testGetElement<F64>("f64", gElemF64);
    testGetElement<bool>("bool", gElemBool);
}

///
/// @brief Region lookup returns the correct pointer.
///
TEST(StateVectorAccess, GetRegion)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    Region* region = nullptr;
    CHECK_SUCCESS(sv.getRegion("foo", region));
    POINTERS_EQUAL(region, &gRegionFoo);
    CHECK_SUCCESS(sv.getRegion("bar", region));
    POINTERS_EQUAL(region, &gRegionBar);
    CHECK_ERROR(E_SV_KEY, sv.getRegion("baz", region));
}

///
/// @brief Looking up an element that exists but whose type doesn't match the
/// provided element pointer returns an error.
///
TEST(StateVectorAccess, ErrorLookUpElementWrongType)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    Element<U32>* elem = nullptr;
    CHECK_ERROR(E_SV_TYPE, sv.getElement("i32", elem));
    CHECK_TRUE(elem == nullptr);
}

///
/// @brief Looking up an element that does not exist returns an error.
///
TEST(StateVectorAccess, ErrorUnknownElement)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    Element<U32>* elem = nullptr;
    CHECK_ERROR(E_SV_KEY, sv.getElement("foo", elem));
    CHECK_TRUE(elem == nullptr);
    IElement* ielem = nullptr;
    CHECK_ERROR(E_SV_KEY, sv.getIElement("foo", ielem));
    CHECK_TRUE(ielem == nullptr);
}

///
/// @brief Looking up a region that does not exist returns an error.
///
TEST(StateVectorAccess, ErrorUnknownRegion)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    Region* region = nullptr;
    CHECK_ERROR(E_SV_KEY, sv.getRegion("baz", region));
    CHECK_TRUE(region == nullptr);
}
