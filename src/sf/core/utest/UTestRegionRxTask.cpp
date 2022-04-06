#include "sf/core/RegionRxTask.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(RegionRxTask)
{
};

TEST(RegionRxTask, Doop)
{
    RegionRxTask<1> task(nullptr, {nullptr, nullptr, nullptr, nullptr, 0, 0});
}
