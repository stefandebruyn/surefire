#include "sf/pal/AnalogIO.hpp"
#include "sf/utest/UTest.hpp"

TEST_GROUP(Sbrio9637AnalogIO)
{
};

TEST(Sbrio9637AnalogIO, SetModeSuccess)
{
    AnalogIO aio;
    CHECK_SUCCESS(AnalogIO::init(aio));

    // Setting all modes on all pins succeeds.
    for (U32 i = 0; i < 16; ++i)
    {
        CHECK_SUCCESS(aio.setMode(i, 0));
        CHECK_SUCCESS(aio.setMode(i, 1));
    }

    // Specifying an invalid pin or mode returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.setMode(16, 0));
    CHECK_ERROR(E_AIO_MODE, aio.setMode(0, 2));
}

TEST(Sbrio9637AnalogIO, SetRangeSuccess)
{
    // Initialize AIO.
    AnalogIO aio;
    CHECK_SUCCESS(AnalogIO::init(aio));

    // Setting all ranges on all pins succeeds.
    for (U32 i = 0; i < 16; ++i)
    {
        CHECK_SUCCESS(aio.setRange(i, 1));
        CHECK_SUCCESS(aio.setRange(i, 2));
        CHECK_SUCCESS(aio.setRange(i, 5));
        CHECK_SUCCESS(aio.setRange(i, 10));
    }

    // Specifying an invalid pin or range returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.setRange(16, 1));
    CHECK_ERROR(E_AIO_RANGE, aio.setRange(0, 3));
}

TEST(Sbrio9637AnalogIO, ReadSuccess)
{
    // Initialize AIO.
    AnalogIO aio;
    CHECK_SUCCESS(AnalogIO::init(aio));

    // Reading all pins succeeds.
    F32 val = 0.0f;
    for (U32 i = 0; i < 16; ++i)
    {
        CHECK_SUCCESS(aio.read(i, val));
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.read(16, val));
}

TEST(Sbrio9637AnalogIO, WriteSuccess)
{
    // Initialize AIO.
    AnalogIO aio;
    CHECK_SUCCESS(AnalogIO::init(aio));

    // Writing all pins succeeds.
    for (U32 i = 0; i < 4; ++i)
    {
        CHECK_SUCCESS(aio.write(i, 0.1f)); // Small voltage
    }

    // Specifying an invalid pin returns an error.
    CHECK_ERROR(E_AIO_PIN, aio.write(4, 0.0f));
}

TEST(Sbrio9637AnalogIO, ReleaseAndReuseSuccess)
{
    AnalogIO aio;
    CHECK_SUCCESS(AnalogIO::init(aio));
    CHECK_SUCCESS(aio.release());
    CHECK_SUCCESS(AnalogIO::init(aio));
}
