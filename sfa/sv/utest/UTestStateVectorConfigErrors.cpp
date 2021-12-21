#include "sfa/sv/StateVector.hpp"
#include "utest/UTest.hpp"

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

TEST_GROUP(StateVectorConfigErrors)
{
};

TEST(StateVectorConfigErrors, NullElementArray)
{
    // Replace element array with nullptr.
    StateVector::ElementConfig* const tmp = gConfig.elems;
    gConfig.elems = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.elems = tmp;
    CHECK_EQUAL(E_NULLPTR, res);

    // State vector is uninitialized.
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}

TEST(StateVectorConfigErrors, NullElementPointer)
{
    // Replace `bar` element pointer with nullptr.
    IElement* const tmp = gConfig.elems[1].elem;
    gConfig.elems[1].elem = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.elems[1].elem = tmp;
    CHECK_EQUAL(E_NULLPTR, res);

    // State vector is uninitialized.
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}

TEST(StateVectorConfigErrors, NullRegionPointer)
{
    // Replace `bar`region pointer with nullptr.
    Region* const tmp = gConfig.regions[1].region;
    gConfig.regions[1].region = nullptr;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.regions[1].region = tmp;
    CHECK_EQUAL(E_NULLPTR, res);

    // State vector is uninitialized.
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}

TEST(StateVectorConfigErrors, MisalignedElementMiddleOfRegion)
{
    // Replace `foo` element with one outside the state vector backing storage.
    I32 backing = 0.0;
    Element<I32> elem(backing);
    IElement* const tmp = gConfig.elems[0].elem;
    gConfig.elems[0].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.elems[0].elem = tmp;
    CHECK_EQUAL(E_LAYOUT, res);

    // State vector is uninitialized.
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}

TEST(StateVectorConfigErrors, MisalignedElementLastInRegion)
{
    // Replace `bar` element with one outside the state vector backing storage.
    F64 backing = 0.0;
    Element<F64> elem(backing);
    IElement* const tmp = gConfig.elems[1].elem;
    gConfig.elems[1].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.elems[1].elem = tmp;

    // State vector is uninitialized.
    CHECK_EQUAL(E_LAYOUT, res);
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}

TEST(StateVectorConfigErrors, MisalignedElementFirstInRegion)
{
    // Replace `baz` element with one outside the state vector backing storage.
    bool backing = 0.0;
    Element<bool> elem(backing);
    IElement* const tmp = gConfig.elems[2].elem;
    gConfig.elems[2].elem = &elem;

    // Creating state vector fails.
    StateVector sv;
    const Result res = StateVector::create(gConfig, sv);
    gConfig.elems[2].elem = tmp;

    // State vector is uninitialized.
    CHECK_EQUAL(E_LAYOUT, res);
    Element<I32>* foo = nullptr;
    CHECK_EQUAL(E_UNINITIALIZED, sv.getElement("foo", foo));
    POINTERS_EQUAL(nullptr, foo);
}
