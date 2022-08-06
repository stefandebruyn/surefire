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
#include "sf/pal/DigitalIO.hpp"

Result DigitalIO::init(DigitalIO& kDio)
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
    (void) this->release();
}

Result DigitalIO::setMode(const U32 kPin, const DigitalIO::Mode kMode)
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    switch (kMode)
    {
        case DigitalIO::IN:
            pinMode(kPin, INPUT);
            break;

        case DigitalIO::OUT:
            pinMode(kPin, OUTPUT);
            break;

        default:
            return E_DIO_MODE;
    }

    return SUCCESS;
}

Result DigitalIO::read(const U32 kPin, bool& kVal)
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    kVal = (digitalRead(kPin) == HIGH);
    return SUCCESS;
}

Result DigitalIO::write(const U32 kPin, const bool kVal)
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

Result DigitalIO::release()
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
