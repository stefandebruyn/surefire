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
/// @file  sf/core/utest/UTestRegion.cpp
/// @brief Unit tests for Region.
////////////////////////////////////////////////////////////////////////////////

#include "sf/core/Region.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Unit tests for Region.
///
TEST_GROUP(Region)
{
};

///
/// @brief Regions are correctly read/written to/from other memory.
///
TEST(Region, ReadWrite)
{
    // Two identical region backings, one with arbitrary initial data and one
    // zeroed-out.
    struct
    {
        I32 i32;
        F64 f64;
        bool b;
    } foo{343, 1.522, true}, bar{};

    // Create region with zeroed backing.
    Region regionBar(&bar, sizeof(bar));

    // Copy backing containing data to region.
    CHECK_SUCCESS(regionBar.write(&foo, sizeof(foo)));

    // Region now contains the same data as the source backing.
    CHECK_EQUAL(343, bar.i32);
    CHECK_EQUAL(1.522, bar.f64);
    CHECK_EQUAL(true, bar.b);

    // Zero the source backing and read the region into it.
    foo = {};
    CHECK_SUCCESS(regionBar.read(&foo, sizeof(foo)));

    // Source backing now contains the same data as the region.
    CHECK_EQUAL(343, foo.i32);
    CHECK_EQUAL(1.522, foo.f64);
    CHECK_EQUAL(true, foo.b);
}

///
/// @brief Region:addr() returns the correct backing address.
///
TEST(Region, GetAddr)
{
    char buf[16];
    Region region(&buf, sizeof(buf));
    POINTERS_EQUAL(buf, region.addr());
}

///
/// @brief Region:size() returns the correct region size in bytes.
///
TEST(Region, GetSize)
{
    char buf[16];
    Region region(&buf, sizeof(buf));
    CHECK_EQUAL(16, region.size());
}
