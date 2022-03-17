#include "sf/pal/Console.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(Console)
{
};

TEST(Console, Printf)
{
    const I32 ret =
        Console::printf("Console print test %s %d %.2f", "foobar", 1522, 9.807);
    CHECK_EQUAL(35, ret);
}
