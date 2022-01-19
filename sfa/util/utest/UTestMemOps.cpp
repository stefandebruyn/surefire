#include "sfa/util/MemOps.hpp"
#include "UTest.hpp"

TEST_GROUP(MemOps)
{
};

TEST(MemOps, StrcmpSingleChars)
{
    CHECK_EQUAL(0, MemOps::strcmp("a", "a"));
    CHECK_EQUAL(('a' - 'A'), MemOps::strcmp("a", "A"));
    CHECK_EQUAL(('A' - 'a'), MemOps::strcmp("A", "a"));
}

TEST(MemOps, StrcmpStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp("foo bar baz", "foo bar baz"));
    CHECK_EQUAL(('b' - 'z'), MemOps::strcmp("foo bar baz", "foo zar baz"));
    CHECK_EQUAL(('z' - 'b'), MemOps::strcmp("foo zar baz", "foo bar baz"));
}

TEST(MemOps, StrcmpEmptyStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp("", ""));
    CHECK_EQUAL('f', MemOps::strcmp("foo", ""));
    CHECK_EQUAL((0 - 'f'), MemOps::strcmp("", "foo"))
}

TEST(MemOps, StrcmpNullStrings)
{
    CHECK_EQUAL(0, MemOps::strcmp(nullptr, nullptr));
    CHECK_EQUAL('f', MemOps::strcmp("foo", nullptr));
    CHECK_EQUAL((0 - 'f'), MemOps::strcmp(nullptr, "foo"));
}

TEST(MemOps, MemcpyOneChar)
{
    U32 src = 0xB3;
    U32 dest = 0;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, &src, 1));
    CHECK_EQUAL(0xB3, src);
    CHECK_EQUAL(src, dest);
}

TEST(MemOps, MemcpyMultipleChars)
{
    U64 src = 0x77FA01B321;
    U64 dest = 0;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, &src, 5));
    CHECK_EQUAL(0x77FA01B321, src);
    CHECK_EQUAL(src, dest);
}

TEST(MemOps, MemcpyDestNull)
{
    U32 src = 0xB3;
    POINTERS_EQUAL(nullptr, MemOps::memcpy(nullptr, &src, 4));
    CHECK_EQUAL(0xB3, src);
}

TEST(MemOps, MemcpySrcNull)
{
    U32 dest = 0xB3;
    POINTERS_EQUAL(&dest, MemOps::memcpy(&dest, nullptr, 4));
    CHECK_EQUAL(0xB3, dest);
}
