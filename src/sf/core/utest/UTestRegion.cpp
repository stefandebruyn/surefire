#include "sf/core/Region.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(Region)
{
};

TEST(Region, ReadWrite)
{
    struct
    {
        I32 i32;
        F64 f64;
        bool b;
    } foo{343, 1.522, true}, bar{};

    Region regionBar(&bar, sizeof(bar));

    CHECK_SUCCESS(regionBar.write(&foo, sizeof(foo)));
    CHECK_EQUAL(343, bar.i32);
    CHECK_EQUAL(1.522, bar.f64);
    CHECK_EQUAL(true, bar.b);

    foo = {};
    CHECK_SUCCESS(regionBar.read(&foo, sizeof(foo)));
    CHECK_EQUAL(343, foo.i32);
    CHECK_EQUAL(1.522, foo.f64);
    CHECK_EQUAL(true, foo.b);
}

TEST(Region, GetAddr)
{
    char buf[16];
    Region region(&buf, sizeof(buf));
    POINTERS_EQUAL(buf, region.addr());
}

TEST(Region, GetSize)
{
    char buf[16];
    Region region(&buf, sizeof(buf));
    CHECK_EQUAL(16, region.size());
}
