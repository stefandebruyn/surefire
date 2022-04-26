#include "sf/core/RealTimeExecutor.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(RealTimeExecutor)
{
};

TEST(RealTimeExecutor, Doop)
{
    RealTimeExecutor<2>::Config config;
    RealTimeExecutor<2> exec(config);
}
