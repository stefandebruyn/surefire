#include <Arduino.h>

#include "sfa/pal/DigitalIo.hpp"

Result DigitalIo::create(DigitalIo& kDio)
{
    if (kDio.mInit == true)
    {
        return E_DIO_REINIT;
    }

    kDio.mInit = true;
    return SUCCESS;
}

DigitalIo::DigitalIo() : mInit(false)
{
}

DigitalIo::~DigitalIo()
{
    (void) this->close();
}

Result DigitalIo::setMode(const U32 kPin, const Mode kMode)
{
    // Verify DIO is initialized.
    if (mInit == false)
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

Result DigitalIo::read(const U32 kPin, bool& kVal)
{
    // Verify DIO is initialized.
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    kVal = (digitalRead(kPin) == HIGH);
    return SUCCESS;
}

Result DigitalIo::write(const U32 kPin, const bool kVal)
{
    // Verify DIO is initialized.
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    if (kVal == true)
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

Result DigitalIo::close()
{
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    Result res = SUCCESS;
    for (U32 i = 0; i < (sizeof(mOutBitVec) * 8); ++i)
    {
        if (((mOutBitVec >> i) & 0x1) == 1)
        {
            const Result writeRes = this->write(i, false);
            if ((writeRes != SUCCESS) && (res == SUCCESS))
            {
                res = writeRes;
            }
        }
    }

    mOutBitVec = 0;
    mInit = false;

    return res;
}
