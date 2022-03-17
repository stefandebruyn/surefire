#include <Arduino.h>

#include "sf/core/Diag.hpp"
#include "sf/pal/DigitalIO.hpp"

Result DigitalIO::create(DigitalIO& kDio)
{
    if (kDio.mInit)
    {
        return E_DIO_REINIT;
    }

    kDio.mInit = true;
    return SUCCESS;
}

DigitalIO::DigitalIO() : mInit(false)
{
}

DigitalIO::~DigitalIO()
{
    (void) this->close();
}

Result DigitalIO::setMode(const U32 kPin, const Mode kMode)
{
    // Verify DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    switch (kMode)
    {
        case IN:
            pinMode(kPin, INPUT);
            break;

        case OUT:
            pinMode(kPin, OUTPUT);
            break;

        default:
            return E_DIO_MODE;
    }

    return SUCCESS;
}

Result DigitalIO::read(const U32 kPin, bool& kVal)
{
    // Verify DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    kVal = (digitalRead(kPin) == HIGH);
    return SUCCESS;
}

Result DigitalIO::write(const U32 kPin, const bool kVal)
{
    // Verify DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    if (kVal)
    {
        digitalWrite(kPin, HIGH);
        mOutBitVec |= (static_cast<U64>(1) << kPin);
    }
    else
    {
        digitalWrite(kPin, LOW);
        mOutBitVec &= ~(static_cast<U64>(1) << kPin);
    }

    return SUCCESS;
}

Result DigitalIO::close()
{
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    Result res = SUCCESS;
    for (U32 i = 0; i < (sizeof(mOutBitVec) * 8); ++i)
    {
        if (((mOutBitVec >> i) & 0x1) == 1)
        {
            const Result writeRes = this->write(i, false);
            Diag::errorStoreCond(writeRes, res);
        }
    }

    mOutBitVec = 0;
    mInit = false;

    return res;
}
