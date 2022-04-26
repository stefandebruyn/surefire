#include "sf/pal/Clock.hpp"
#include "sf/pal/DigitalIO.hpp"
#include "sf/utest/UTest.hpp"

static const U32 gDigitalPinCnt = 28;

TEST_GROUP(NilrtDigitalIO)
{
};

TEST(NilrtDigitalIO, RaiseAllLowerAll)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Set all pins as output and raise them.
    for (U32 i = 0; i < gDigitalPinCnt; ++i)
    {
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

TEST(NilrtDigitalIO, ReleaseAndReuse)
{
    // Initialize DIO.
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Raise pin 0.
    CHECK_SUCCESS(dio.write(0, true));

    // Release DIO.
    CHECK_SUCCESS(dio.release());

    // Attempting to write the DIO again returns an error.
    CHECK_ERROR(E_DIO_UNINIT, dio.write(0, true));

    // Reinitialize DIO.
    CHECK_SUCCESS(DigitalIO::init(dio));

    // Raise pin 0.
    CHECK_SUCCESS(dio.write(0, true));
}

// TEST(NilrtDigitalIO, 
