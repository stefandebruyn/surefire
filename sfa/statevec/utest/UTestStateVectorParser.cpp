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
    ConfigInfo configInfo;
    Result res = StateVectorParser::parse(
        "/home/srd2557/thesis/derp.txt", config, &configInfo);
    std::cout << res << std::endl;
    if (res != SUCCESS)
    {
        std::cout << configInfo.prettifyError() << std::endl;
    }
}
