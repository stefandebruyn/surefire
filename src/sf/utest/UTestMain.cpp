////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
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
/// @file  sf/utest/UTestMain.cpp
/// @brief Unit test entry point.
////////////////////////////////////////////////////////////////////////////////

#include "CppUTest/CommandLineTestRunner.h"

#ifdef SF_PLATFORM_SBRIO9637
#    include "sf/psl/sbrio9637/NiFpgaSession.hpp"
#endif

int main(int argc, char* argv[])
{
#ifdef SF_PLATFORM_SBRIO9637
    // sbRIO-9637: Force the FPGA library to load before running tests so that
    // it doesn't trigger the memory leak detector.
    // https://www.ni.com/en-us/support/documentation/bugs/18/labview-2018-fpga-module-known-issues.html#660205_by_Date
    NiFpga_Session session;
    (void) Sf::niFpgaSessionOpen(session);
    (void) Sf::niFpgaSessionClose(session);
#endif

    return CommandLineTestRunner::RunAllTests(argc, argv);
}
