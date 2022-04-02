#include "sf/core/StateVector.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Globals ////////////////////////////////////

// Test state vector backing storage.
#pragma pack(push, 1)
static struct
{
    struct
    {
        I32 foo;
        F64 bar;
    } foo;
    struct
    {
        bool baz;
    } bar;
} gBacking;
#pragma pack(pop)

// Test state vector elements.
static Element<I32> gElemFoo(gBacking.foo.foo);
static Element<F64> gElemBar(gBacking.foo.bar);
static Element<bool> gElemBaz(gBacking.bar.baz);

// Test state vector element configs.
static StateVector::ElementConfig gElems[] =
{
    {"foo", &gElemFoo},
    {"bar", &gElemBar},
    {"baz", &gElemBaz},
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

static void checkStateVectorUninitialized(StateVector& kSv)
{
    // Getting an element fails.
    Element<I32>* elem = nullptr;
    CHECK_ERROR(E_SV_UNINIT, kSv.getElement("foo", elem));
    POINTERS_EQUAL(nullptr, elem);

    // Getting a region fails.
    Region* region = nullptr;
    CHECK_ERROR(E_SV_UNINIT, kSv.getRegion("foo", region));
    POINTERS_EQUAL(nullptr, elem);
}

//////////////////////////////////// Tests /////////////////////////////////////

TEST_GROUP(StateVectorCreate)
{
    void teardown()
    {
        // `AllowElementMisalignmentWithoutRegions` can fail without restoring
        // the `bar` element pointer in `gConfig`, so do that here.
        gConfig.elems[1].elem = &gElemBar;
    }
};

TEST(StateVectorCreate, Success)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
}

TEST(StateVectorCreate, Uninitialized)
{
    StateVector sv;
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorReinitialize)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    CHECK_ERROR(E_SV_REINIT, StateVector::init(gConfig, sv));
}

TEST(StateVectorCreate, ErrorNullElementArray)
{
    // Replace element array with nullptr.
    StateVector::ElementConfig* const tmp = gConfig.elems;
    gConfig.elems = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems = tmp;
    CHECK_ERROR(E_SV_NULL, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorNullElementPointer)
{
    // Replace element `bar` pointer with nullptr.
    IElement* const tmp = gConfig.elems[1].elem;
    gConfig.elems[1].elem = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems[1].elem = tmp;
    CHECK_ERROR(E_SV_NULL, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorNullRegionPointer)
{
    // Replace region `bar` pointer with nullptr.
    Region* const tmp = gConfig.regions[1].region;
    gConfig.regions[1].region = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.regions[1].region = tmp;
    CHECK_ERROR(E_SV_NULL, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorMisalignedElementMiddleOfRegion)
{
    // Replace element `foo` with one outside the state vector backing storage.
    I32 backing = 0.0;
    Element<I32> elem(backing);
    IElement* const tmp = gConfig.elems[0].elem;
    gConfig.elems[0].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems[0].elem = tmp;
    CHECK_ERROR(E_SV_LAYOUT, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorMisalignedElementLastInRegion)
{
    // Replace element `bar` with one outside the state vector backing storage.
    F64 backing = 0.0;
    Element<F64> elem(backing);
    IElement* const tmp = gConfig.elems[1].elem;
    gConfig.elems[1].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems[1].elem = tmp;
    CHECK_ERROR(E_SV_LAYOUT, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, ErrorMisalignedElementFirstInRegion)
{
    // Replace element `baz` with one outside the state vector backing storage.
    bool backing = 0.0;
    Element<bool> elem(backing);
    IElement* const tmp = gConfig.elems[2].elem;
    gConfig.elems[2].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems[2].elem = tmp;
    CHECK_ERROR(E_SV_LAYOUT, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

// Non-contiguous elements are allowed when not using regions.
TEST(StateVectorCreate, AllowElementMisalignmentWithoutRegions)
{
    // Replace element `bar` with one outside the state vector backing storage
    // and null out the regions array.
    F64 backing = 0.0;
    Element<F64> elem(backing);
    IElement* const tmp0 = gConfig.elems[1].elem;
    gConfig.elems[1].elem = &elem;
    StateVector::RegionConfig* const tmp1 = gConfig.regions;
    gConfig.regions = nullptr;

    // Create state vector.
    StateVector sv;
    Result res = StateVector::init(gConfig, sv);
    gConfig.regions = tmp1;
    CHECK_SUCCESS(res);

    // Getting element `bar` returns the element created locally above.
    Element<F64>* elemBar = nullptr;
    res = sv.getElement("bar", elemBar);
    gConfig.elems[1].elem = tmp0;
    CHECK_SUCCESS(res);
    POINTERS_EQUAL(&elem, elemBar);
}

TEST(StateVectorCreate, DupeElementName)
{
    // Rename element `bar` to `foo`.
    const char* const tmp = gConfig.elems[1].name;
    gConfig.elems[1].name = "foo";

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.elems[1].name = tmp;
    CHECK_ERROR(E_SV_ELEM_DUPE, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}

TEST(StateVectorCreate, DupeRegionName)
{
    // Rename region `bar` to `foo`.
    const char* const tmp = gConfig.regions[1].name;
    gConfig.regions[1].name = "foo";

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::init(gConfig, sv);
    gConfig.regions[1].name = tmp;
    CHECK_ERROR(E_SV_RGN_DUPE, res);

    // State vector is uninitialized.
    checkStateVectorUninitialized(sv);
}
