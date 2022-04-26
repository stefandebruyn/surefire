#include "CppUTest/CommandLineTestRunner.h"

#ifdef SF_PLATFORM_SBRIO9637
#    include "sf/psl/sbrio9637/NiFpgaSession.hpp"
#endif

int main(int argc, char* argv[])
{
#ifdef SF_PLATFORM_SBRIO9637
    // sbRIO-9637: Force the FPGA library to load before running tests so that
    // it doesn't trigger the memory leak detector.
    NiFpga_Session session;
    (void) niFpgaSessionOpen(session);
    (void) niFpgaSessionClose(session);
#endif

    return CommandLineTestRunner::RunAllTests(argc, argv);
}
