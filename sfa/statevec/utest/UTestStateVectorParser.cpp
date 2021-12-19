#include "sfa/statevec/StateVectorParser.hpp"
#include "CppUTest/TestHarness.h"

#include <iostream> // rm later

TEST_GROUP(StateVectorParser)
{
};

TEST(StateVectorParser, Parse)
{
    StateVector::Config config;
    std::cout << std::endl;
    ConfigErrorInfo errInfo;
    Result res = StateVectorParser::parse(
        "/home/srd2557/thesis/derp.txt", config, &errInfo);
    std::cout << res << std::endl;
    std::cout << errInfo.prettify() << std::endl;
}
