#include "CppUTest/CommandLineTestRunner.h"

#ifdef SF_PLATFORM_NILRT
#    include "sf/psl/nilrt/NiFpgaSession.hpp"
#endif

int main(int argc, char* argv[])
{
#ifdef SF_PLATFORM_NILRT
    // NILRT: Force the FPGA library to load before running tests so that it
    // doesn't trigger the memory leak detector.
    NiFpga_Session session;
    (void) niFpgaSessionOpen(session);
    (void) niFpgaSessionClose(session);
#endif

    return CommandLineTestRunner::RunAllTests(argc, argv);
}
