#include "sf/pal/Clock.hpp"
#include "sf/pal/DigitalIO.hpp"
#include "sf/utest/UTest.hpp"

static const U32 gDigitalPinCnt = 28;

TEST_GROUP(NilrtDigitalIO)
{
};

TEST(NilrtDigitalIO, SetModeSuccess)
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

TEST(NilrtDigitalIO, ReadSuccess)
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

TEST(NilrtDigitalIO, WriteSuccess)
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

TEST(NilrtDigitalIO, ReleaseAndReuseSuccess)
{
    DigitalIO dio;
    CHECK_SUCCESS(DigitalIO::init(dio));
    CHECK_SUCCESS(dio.release());
    CHECK_SUCCESS(DigitalIO::init(dio));
}

TEST(NilrtDigitalIO, RaiseAndLower)
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
