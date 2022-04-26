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
/// @file  sf/core/utest/UTestStateVectorInit.cpp
/// @brief Unit tests for StateVector::init().
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/StateVector.hpp"
#include "sf/utest/UTest.hpp"

/////////////////////////////////// Global /////////////////////////////////////

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

///
/// @brief Checks that a StateVector is uninitialized by invoking all its
/// methods and expecting E_SV_UNINIT.
///
/// @param[in] kSv  State vector to check.
///
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

///
/// @brief Unit tests for StateVector::init().
///
TEST_GROUP(StateVectorInit)
{
    void teardown()
    {
        // `AllowElementMisalignmentWithoutRegions` can fail without restoring
        // the `bar` element pointer in `gConfig`, so do that here.
        gConfig.elems[1].elem = &gElemBar;
    }
};

///
/// @test State vector initialization succeeds with a valid config.
///
TEST(StateVectorInit, Success)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
}

///
/// @test A default-constructed state vector is uninitialized.
///
TEST(StateVectorInit, Uninitialized)
{
    StateVector sv;
    checkStateVectorUninitialized(sv);
}

///
/// @test Initializing a state vector twice fails.
///
TEST(StateVectorInit, ErrorReinitialize)
{
    StateVector sv;
    CHECK_SUCCESS(StateVector::init(gConfig, sv));
    CHECK_ERROR(E_SV_REINIT, StateVector::init(gConfig, sv));
}

///
/// @test Initializing a state vector with a null element array returns an
/// error.
///
TEST(StateVectorInit, ErrorNullElementArray)
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

///
/// @test Initializing a state vector with a null element pointer in one of the
/// element configs returns an error.
///
TEST(StateVectorInit, ErrorNullElementPointer)
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

///
/// @test Initializing a state vector with a null region pointer in one of the
/// region configs returns an error.
///
TEST(StateVectorInit, ErrorNullRegionPointer)
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

///
/// @test Initializing a state vector with an element in the middle of a region
/// having backing memory outside the region returns an error.
///
TEST(StateVectorInit, ErrorMisalignedElementMiddleOfRegion)
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

///
/// @test Initializing a state vector with the last element in a region having
/// backing memory outside the region returns an error.
///
TEST(StateVectorInit, ErrorMisalignedElementLastInRegion)
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

///
/// @test Initializing a state vector with the first element in a region having
/// backing memory outside the region returns an error.
///
TEST(StateVectorInit, ErrorMisalignedElementFirstInRegion)
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

///
/// @test Element backing memory alignment is not enforced when the state vector
/// is configured without regions.
///
TEST(StateVectorInit, AllowElementMisalignmentWithoutRegions)
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

///
/// @test Initializing a state vector that uses the same element name twice
/// return an error.
///
TEST(StateVectorInit, DupeElementName)
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

///
/// @test Initializing a state vector that uses the same region name twice
/// return an error.
///
TEST(StateVectorInit, DupeRegionName)
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
