////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
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
/// @file  sf/psl/sbrio9637/utest/UTestSbrio9637DigitalIO.cpp
/// @brief Unit tests for DigitalIO on sbRIO-9637.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/Clock.hpp"
#include "sf/pal/DigitalIO.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Number of digital pins on sbRIO-9637.
///
static const U32 gDigitalPinCnt = 28;

///
/// @brief Unit tests for DigitalIO on sbRIO-9637.
///
TEST_GROUP(Sbrio9637DigitalIO)
{
};

///
/// @test Setting all modes on all digital pins succeeds.
///
TEST(Sbrio9637DigitalIO, SetModeSuccess)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Setting all modes on all pins succeeds.
    for (U32 i = 0; i < 28; ++i)
    {
        CHECK_SUCCESS(dio.setMode(i, DigitalIO::IN));
        CHECK_SUCCESS(dio.setMode(i, DigitalIO::OUT));
    }

    // Specifying an invalid pin or mode returns an error.
    CHECK_ERROR(E_DIO_PIN, dio.setMode(28, DigitalIO::IN));
    CHECK_ERROR(E_DIO_MODE, dio.setMode(0, static_cast<DigitalIO::Mode>(2)));
}

///
/// @test Reading all digital pins succeeds.
///
TEST(Sbrio9637DigitalIO, ReadSuccess)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Reading all pins succeeds.
    bool val = false;
    for (U32 i = 0; i < 28; ++i)
    {
        CHECK_SUCCESS(dio.read(i, val));
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_DIO_PIN, dio.read(28, val));
}

///
/// @test Writing all digital pins succeeds.
///
TEST(Sbrio9637DigitalIO, WriteSuccess)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Writing low and high to all pins succeeds.
    for (U32 i = 0; i < 28; ++i)
    {
        CHECK_SUCCESS(dio.write(i, false));
        CHECK_SUCCESS(dio.write(i, true));
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_DIO_PIN, dio.write(28, false));
}

///
/// @test DigitalIO can be reinitialized after releasing.
///
TEST(Sbrio9637DigitalIO, ReleaseAndReuseSuccess)
{
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));
    CHECK_SUCCESS(dio.release());
    CHECK_SUCCESS(DigitalIO::init(dio));
}

///
/// @test Digital pins read high after being raised, and low after being
/// lowered.
///
TEST(Sbrio9637DigitalIO, RaiseAndLower)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Set all pins as output and raise them.
    for (U32 i = 0; i < gDigitalPinCnt; ++i)
    {
        CHECK_SUCCESS(dio.setMode(i, DigitalIO::IN));
        CHECK_SUCCESS(dio.setMode(i, DigitalIO::OUT));
        CHECK_SUCCESS(dio.write(i, true));
    }

    // Wait a relatively long time to avoid racing the FPGA.
    Clock::spinWait(Clock::NS_IN_S * 0.1);

    // All pins read high.
    for (U32 i = 0; i < gDigitalPinCnt; ++i)
    {
        bool val = false;
        CHECK_SUCCESS(dio.read(i, val));
        CHECK_TRUE(val);
    }

    // Lower all pins.
    for (U32 i = 0; i < gDigitalPinCnt; ++i)
    {
        CHECK_SUCCESS(dio.write(i, false));
    }

    // Wait a relatively long time to avoid racing the FPGA.
    Clock::spinWait(Clock::NS_IN_S * 0.1);

    // All pins read low.
    for (U32 i = 0; i < gDigitalPinCnt; ++i)
    {
        bool val = true;
        CHECK_SUCCESS(dio.read(i, val));
        CHECK_TRUE(!val);
    }
}
