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
////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include "sf/core/Diagnostic.hpp"
#include "sf/pal/DigitalIo.hpp"

Result DigitalIo::init(DigitalIo& kDio)
{
    if (kDio.mInit)
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
    (void) this->release();
}

Result DigitalIo::setMode(const U32 kPin, const DigitalIo::Mode kMode)
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    switch (kMode)
    {
        case DigitalIo::IN:
            pinMode(kPin, INPUT);
            break;

        case DigitalIo::OUT:
            pinMode(kPin, OUTPUT);
            break;

        default:
            return E_DIO_MODE;
    }

    return SUCCESS;
}

Result DigitalIo::read(const U32 kPin, bool& kVal)
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    kVal = (digitalRead(kPin) == HIGH);
    return SUCCESS;
}

Result DigitalIo::write(const U32 kPin, const bool kVal)
{
    // Check that DIO is initialized.
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

Result DigitalIo::release()
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
            Diag::errsc(writeRes, res);
        }
    }

    mOutBitVec = 0;
    mInit = false;

    return res;
}
