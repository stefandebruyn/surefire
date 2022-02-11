#include <Arduino.h>

#include "sfa/pal/Dio.hpp"

Result Dio::create(Dio& kDio)
{
    if (kDio.mInit == true)
    {
        return E_DIO_REINIT;
    }

    kDio.mInit = true;
    return SUCCESS;
}

Dio::Dio() : mInit(false)
{
}

Dio::~Dio()
{
    (void) this->close();
}

Result Dio::setMode(const U32 kPin, const Mode kMode)
{
    // Verify DIO is initialized.
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    switch (kMode)
    {
        case MODE_INPUT:
            pinMode(kPin, INPUT);
            break;

        case MODE_OUTPUT:
            pinMode(kPin, OUTPUT);
            break;

        default:
            return E_DIO_MODE;
    }

    return SUCCESS;
}

Result Dio::read(const U32 kPin, bool& kVal)
{
    // Verify DIO is initialized.
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    kVal = (digitalRead(kPin) == HIGH);
    return SUCCESS;
}

Result Dio::write(const U32 kPin, const bool kVal)
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

Result Dio::close()
{
    if (mInit == false)
    {
        return E_DIO_UNINIT;
    }

    Result res = SUCCESS;
    for (U32 i = 0; i < sizeof(mOutBitVec); ++i)
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
