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
/// @file  sf/psl/sbrio9637/utest/UTestSbrio9637AnalogIo.cpp
/// @brief Unit tests for AnalogIo on sbRIO-9637.
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/AnalogIo.hpp"
#include "sf/utest/UTest.hpp"

///
/// @brief Number of analog inputs on sbRIO-9637.
///
static const U32 gAinCnt = 16;

///
/// @brief Number of analog outputs on sbRIO-9637.
///
static const U32 gAoutCnt = 4;

///
/// @brief Unit tests for AnalogIo on sbRIO-9637.
///
/// @remark Testing the hardware functionality of the analog I/O requires
/// external wiring work, so these tests are mostly a smoketest to check that
/// the AnalogIo interface works as expected.
/// 
TEST_GROUP(Sbrio9637AnalogIo)
{
};

///
/// @test Setting all modes on all analog inputs succeeds.
///
TEST(Sbrio9637AnalogIo, SetModeSuccess)
{
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));

    // Setting all modes on all pins succeeds.
    for (U32 i = 0; i < gAinCnt; ++i)
    {
        CHECK_SUCCESS(aio.setMode(i, 0));
        CHECK_SUCCESS(aio.setMode(i, 1));
    }

    // Specifying an invalid pin or mode returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.setMode(gAinCnt, 0));
    CHECK_ERROR(E_AIO_MODE, aio.setMode(0, 2));
}

///
/// @test Setting all ranges on all analog inputs succeeds.
///
TEST(Sbrio9637AnalogIo, SetRangeSuccess)
{
    // Initialize AIO.
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));

    // Setting all ranges on all pins succeeds.
    for (U32 i = 0; i < gAinCnt; ++i)
    {
        CHECK_SUCCESS(aio.setRange(i, 1));
        CHECK_SUCCESS(aio.setRange(i, 2));
        CHECK_SUCCESS(aio.setRange(i, 5));
        CHECK_SUCCESS(aio.setRange(i, 10));
    }

    // Specifying an invalid pin or range returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.setRange(gAinCnt, 1));
    CHECK_ERROR(E_AIO_RANGE, aio.setRange(0, 3));
}

///
/// @test Reading all analog inputs succeeds.
///
TEST(Sbrio9637AnalogIo, ReadSuccess)
{
    // Initialize AIO.
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));

    // Reading all pins succeeds.
    F32 val = 0.0f;
    for (U32 i = 0; i < gAinCnt; ++i)
    {
        CHECK_SUCCESS(aio.read(i, val));
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.read(gAinCnt, val));
}

///
/// @test Writing all analog outputs succeeds.
///
TEST(Sbrio9637AnalogIo, WriteSuccess)
{
    // Initialize AIO.
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));

    // Writing all pins succeeds.
    for (U32 i = 0; i < gAoutCnt; ++i)
    {
        CHECK_SUCCESS(aio.write(i, 0.1f)); // Small voltage
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.write(4, 0.0f));
}

///
/// @test AnalogIo can be reinitialized after releasing.
///
TEST(Sbrio9637AnalogIo, ReleaseAndReuseSuccess)
{
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));
    CHECK_SUCCESS(aio.release());
    CHECK_SUCCESS(AnalogIo::init(aio));
}

///
/// @test Writing a value outside the analog output range returns an error.
///
TEST(Sbrio9637AnalogIo, ErrorOutputOutOfRange)
{
    AnalogIo aio;
    CHECK_SUCCESS(AnalogIo::init(aio));
    CHECK_ERROR(E_AIO_OUT, aio.write(0, -10.1f));
    CHECK_ERROR(E_AIO_OUT, aio.write(0, 10.1f));
}
