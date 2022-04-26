#include <cstring>

#include "sf/core/Diagnostic.hpp"
#include "sf/pal/DigitalIO.hpp"

// FPGA API has its own bool type - assert that this type is equivalent to C++
// bool to simplify things later.
static_assert(sizeof(NiFpga_Bool) == sizeof(bool),
              "NiFpga_Bool unexpected size");
static_assert(NiFpga_True == true, "NiFpga_True unexpected value");
static_assert(NiFpga_False == false, "NiFpga_False unexpected value");

static const U32 gDigitalPinCnt = 28;

static const NiFpga_IO_IndicatorBool gDinIds[gDigitalPinCnt] =
{
    NiFpga_IO_IndicatorBool_inDIO0,
    NiFpga_IO_IndicatorBool_inDIO1,
    NiFpga_IO_IndicatorBool_inDIO2,
    NiFpga_IO_IndicatorBool_inDIO3,
    NiFpga_IO_IndicatorBool_inDIO4,
    NiFpga_IO_IndicatorBool_inDIO5,
    NiFpga_IO_IndicatorBool_inDIO6,
    NiFpga_IO_IndicatorBool_inDIO7,
    NiFpga_IO_IndicatorBool_inDIO8,
    NiFpga_IO_IndicatorBool_inDIO9,
    NiFpga_IO_IndicatorBool_inDIO10,
    NiFpga_IO_IndicatorBool_inDIO11,
    NiFpga_IO_IndicatorBool_inDIO12,
    NiFpga_IO_IndicatorBool_inDIO13,
    NiFpga_IO_IndicatorBool_inDIO14,
    NiFpga_IO_IndicatorBool_inDIO15,
    NiFpga_IO_IndicatorBool_inDIO16,
    NiFpga_IO_IndicatorBool_inDIO17,
    NiFpga_IO_IndicatorBool_inDIO18,
    NiFpga_IO_IndicatorBool_inDIO19,
    NiFpga_IO_IndicatorBool_inDIO20,
    NiFpga_IO_IndicatorBool_inDIO21,
    NiFpga_IO_IndicatorBool_inDIO22,
    NiFpga_IO_IndicatorBool_inDIO23,
    NiFpga_IO_IndicatorBool_inDIO24,
    NiFpga_IO_IndicatorBool_inDIO25,
    NiFpga_IO_IndicatorBool_inDIO26,
    NiFpga_IO_IndicatorBool_inDIO27
};

static const NiFpga_IO_ControlBool gDoutIds[gDigitalPinCnt] =
{
    NiFpga_IO_ControlBool_outDIO0,
    NiFpga_IO_ControlBool_outDIO1,
    NiFpga_IO_ControlBool_outDIO2,
    NiFpga_IO_ControlBool_outDIO3,
    NiFpga_IO_ControlBool_outDIO4,
    NiFpga_IO_ControlBool_outDIO5,
    NiFpga_IO_ControlBool_outDIO6,
    NiFpga_IO_ControlBool_outDIO7,
    NiFpga_IO_ControlBool_outDIO8,
    NiFpga_IO_ControlBool_outDIO9,
    NiFpga_IO_ControlBool_outDIO10,
    NiFpga_IO_ControlBool_outDIO11,
    NiFpga_IO_ControlBool_outDIO12,
    NiFpga_IO_ControlBool_outDIO13,
    NiFpga_IO_ControlBool_outDIO14,
    NiFpga_IO_ControlBool_outDIO15,
    NiFpga_IO_ControlBool_outDIO16,
    NiFpga_IO_ControlBool_outDIO17,
    NiFpga_IO_ControlBool_outDIO18,
    NiFpga_IO_ControlBool_outDIO19,
    NiFpga_IO_ControlBool_outDIO20,
    NiFpga_IO_ControlBool_outDIO21,
    NiFpga_IO_ControlBool_outDIO22,
    NiFpga_IO_ControlBool_outDIO23,
    NiFpga_IO_ControlBool_outDIO24,
    NiFpga_IO_ControlBool_outDIO25,
    NiFpga_IO_ControlBool_outDIO26,
    NiFpga_IO_ControlBool_outDIO27
};

static const NiFpga_IO_ControlBool gDoutEnableIds[gDigitalPinCnt] =
{
    NiFpga_IO_ControlBool_outputEnableDIO0,
    NiFpga_IO_ControlBool_outputEnableDIO1,
    NiFpga_IO_ControlBool_outputEnableDIO2,
    NiFpga_IO_ControlBool_outputEnableDIO3,
    NiFpga_IO_ControlBool_outputEnableDIO4,
    NiFpga_IO_ControlBool_outputEnableDIO5,
    NiFpga_IO_ControlBool_outputEnableDIO6,
    NiFpga_IO_ControlBool_outputEnableDIO7,
    NiFpga_IO_ControlBool_outputEnableDIO8,
    NiFpga_IO_ControlBool_outputEnableDIO9,
    NiFpga_IO_ControlBool_outputEnableDIO10,
    NiFpga_IO_ControlBool_outputEnableDIO11,
    NiFpga_IO_ControlBool_outputEnableDIO12,
    NiFpga_IO_ControlBool_outputEnableDIO13,
    NiFpga_IO_ControlBool_outputEnableDIO14,
    NiFpga_IO_ControlBool_outputEnableDIO15,
    NiFpga_IO_ControlBool_outputEnableDIO16,
    NiFpga_IO_ControlBool_outputEnableDIO17,
    NiFpga_IO_ControlBool_outputEnableDIO18,
    NiFpga_IO_ControlBool_outputEnableDIO19,
    NiFpga_IO_ControlBool_outputEnableDIO20,
    NiFpga_IO_ControlBool_outputEnableDIO21,
    NiFpga_IO_ControlBool_outputEnableDIO22,
    NiFpga_IO_ControlBool_outputEnableDIO23,
    NiFpga_IO_ControlBool_outputEnableDIO24,
    NiFpga_IO_ControlBool_outputEnableDIO25,
    NiFpga_IO_ControlBool_outputEnableDIO26,
    NiFpga_IO_ControlBool_outputEnableDIO27
};

Result DigitalIO::init(DigitalIO& kDio)
{
    if (kDio.mInit)
    {
        return E_DIO_REINIT;
    }

    const Result res = niFpgaSessionOpen(kDio.mSession);
    if (res != SUCCESS)
    {
        return res;
    }

    kDio.mInit = true;

    return SUCCESS;
}

DigitalIO::DigitalIO() : mInit(false), mSession(0)
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

    // Check that pin number is in range.
    if (kPin >= gDigitalPinCnt)
    {
        return E_DIO_PIN;
    }

    // Map digital mode onto FPGA API constant.
    NiFpga_Bool outputEnable = 0;
    switch (kMode)
    {
        case DigitalIO::IN:
            outputEnable = NiFpga_False;
            break;

        case DigitalIO::OUT:
            outputEnable = NiFpga_True;
            break;

        default:
            return E_DIO_MODE;
    }

    // Set pin mode.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat, NiFpga_WriteBool(mSession,
                                               gDoutEnableIds[kPin],
                                               outputEnable));
    if (stat != NiFpga_Status_Success)
    {
        return E_NI_FPGA_DMODE;
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

    // Check that pin number is in range.
    if (kPin >= gDigitalPinCnt)
    {
        return E_DIO_PIN;
    }

    // Do read.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat,
                       NiFpga_ReadBool(mSession,
                                       gDinIds[kPin],
                                       reinterpret_cast<NiFpga_Bool*>(&kVal)));
    if (stat != NiFpga_Status_Success)
    {
        return E_DIO_READ;
    }

    return SUCCESS;
}

Result DigitalIO::write(const U32 kPin, const bool kVal)
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    // Check that pin number is in range.
    if (kPin >= gDigitalPinCnt)
    {
        return E_DIO_PIN;
    }

    // Do write.
    NiFpga_Status stat = NiFpga_Status_Success;
    const NiFpga_Bool writeVal = (kVal ? NiFpga_True : NiFpga_False);
    NiFpga_MergeStatus(&stat,
                       NiFpga_WriteBool(mSession, gDoutIds[kPin], writeVal));
    if (stat != NiFpga_Status_Success)
    {
        return E_DIO_READ;
    }

    return SUCCESS;
}

Result DigitalIO::release()
{
    // Check that DIO is initialized.
    if (!mInit)
    {
        return E_DIO_UNINIT;
    }

    // Close FPGA session.
    const Result res = niFpgaSessionClose(mSession);
    if (res != SUCCESS)
    {
        return res;
    }

    mInit = false;

    return SUCCESS;
}
