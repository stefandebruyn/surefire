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
////////////////////////////////////////////////////////////////////////////////

#include "sf/pal/AnalogIo.hpp"

//////////////////////////////// Private Data //////////////////////////////////

///
/// @brief Number of analog input pins on sbRIO-9637.
///
static constexpr U32 gAiCnt = 16;

///
/// @brief Number of analog output pins on sbRIO-9637.
///
static constexpr U32 gAoCnt = 4;

///
/// @brief FPGA API constants for setting AI modes, indexed by pin.
///
static const NiFpga_IO_ControlU8 gAiModeIds[gAiCnt] =
{
    NiFpga_IO_ControlU8_modeAI0,
    NiFpga_IO_ControlU8_modeAI1,
    NiFpga_IO_ControlU8_modeAI2,
    NiFpga_IO_ControlU8_modeAI3,
    NiFpga_IO_ControlU8_modeAI4,
    NiFpga_IO_ControlU8_modeAI5,
    NiFpga_IO_ControlU8_modeAI6,
    NiFpga_IO_ControlU8_modeAI7,
    NiFpga_IO_ControlU8_modeAI8,
    NiFpga_IO_ControlU8_modeAI9,
    NiFpga_IO_ControlU8_modeAI10,
    NiFpga_IO_ControlU8_modeAI11,
    NiFpga_IO_ControlU8_modeAI12,
    NiFpga_IO_ControlU8_modeAI13,
    NiFpga_IO_ControlU8_modeAI14,
    NiFpga_IO_ControlU8_modeAI15
};

///
/// @brief FPGA API constants for setting AI ranges, indexed by pin.
///
static const NiFpga_IO_ControlU8 gAiRangeIds[gAiCnt] =
{
    NiFpga_IO_ControlU8_rangeAI0,
    NiFpga_IO_ControlU8_rangeAI1,
    NiFpga_IO_ControlU8_rangeAI2,
    NiFpga_IO_ControlU8_rangeAI3,
    NiFpga_IO_ControlU8_rangeAI4,
    NiFpga_IO_ControlU8_rangeAI5,
    NiFpga_IO_ControlU8_rangeAI6,
    NiFpga_IO_ControlU8_rangeAI7,
    NiFpga_IO_ControlU8_rangeAI8,
    NiFpga_IO_ControlU8_rangeAI9,
    NiFpga_IO_ControlU8_rangeAI10,
    NiFpga_IO_ControlU8_rangeAI11,
    NiFpga_IO_ControlU8_rangeAI12,
    NiFpga_IO_ControlU8_rangeAI13,
    NiFpga_IO_ControlU8_rangeAI14,
    NiFpga_IO_ControlU8_rangeAI15
};

///
/// @brief FPGA API constants for reading AI fixed points, indexed by pin.
///
static const uint32_t gAiFxpResourceIds[gAiCnt] =
{
    NiFpga_IO_IndicatorFxp_inputAI0_Resource,
    NiFpga_IO_IndicatorFxp_inputAI1_Resource,
    NiFpga_IO_IndicatorFxp_inputAI2_Resource,
    NiFpga_IO_IndicatorFxp_inputAI3_Resource,
    NiFpga_IO_IndicatorFxp_inputAI4_Resource,
    NiFpga_IO_IndicatorFxp_inputAI5_Resource,
    NiFpga_IO_IndicatorFxp_inputAI6_Resource,
    NiFpga_IO_IndicatorFxp_inputAI7_Resource,
    NiFpga_IO_IndicatorFxp_inputAI8_Resource,
    NiFpga_IO_IndicatorFxp_inputAI9_Resource,
    NiFpga_IO_IndicatorFxp_inputAI10_Resource,
    NiFpga_IO_IndicatorFxp_inputAI11_Resource,
    NiFpga_IO_IndicatorFxp_inputAI12_Resource,
    NiFpga_IO_IndicatorFxp_inputAI13_Resource,
    NiFpga_IO_IndicatorFxp_inputAI14_Resource,
    NiFpga_IO_IndicatorFxp_inputAI15_Resource
};

///
/// @brief FPGA API constants for converting AI fixed points to volts, indexed
/// by pin.
///
static const NiFpga_FxpTypeInfo gAiFxpTypeInfoIds[gAiCnt] =
{
    NiFpga_IO_IndicatorFxp_inputAI0_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI1_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI2_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI3_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI4_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI5_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI6_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI7_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI8_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI9_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI10_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI11_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI12_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI13_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI14_TypeInfo,
    NiFpga_IO_IndicatorFxp_inputAI15_TypeInfo
};

///
/// @brief FPGA API constants for writing AO fixed points, indexed by pin.
///
static const uint32_t gAoFxpResourceIds[gAoCnt] =
{
    NiFpga_IO_ControlFxp_outputAO0_Resource,
    NiFpga_IO_ControlFxp_outputAO1_Resource,
    NiFpga_IO_ControlFxp_outputAO2_Resource,
    NiFpga_IO_ControlFxp_outputAO3_Resource
};

///
/// @brief FPGA API constants for converting volts to AO fixed points, indexed
/// by pin.
///
static const NiFpga_FxpTypeInfo gAoFxpTypeInfoIds[gAoCnt] =
{
    NiFpga_IO_ControlFxp_outputAO0_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO1_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO2_TypeInfo,
    NiFpga_IO_ControlFxp_outputAO3_TypeInfo,
};

/////////////////////////////////// Public /////////////////////////////////////

Result AnalogIo::init(AnalogIo& kAio)
{
    // Check that AIO is not already initialized.
    if (kAio.mInit)
    {
        return E_DIO_REINIT;
    }

    // Open FPGA session.
    const Result res = niFpgaSessionOpen(kAio.mSession);
    if (res != SUCCESS)
    {
        return res;
    }

    kAio.mInit = true;

    return SUCCESS;
}

AnalogIo::AnalogIo() : mInit(false), mSession(0)
{
}

AnalogIo::~AnalogIo()
{
    (void) this->release();
}

Result AnalogIo::setMode(const U32 kPin, const I8 kMode)
{
    // Check that AIO is initialized.
    if (!mInit)
    {
        return E_AIO_UNINIT;
    }

    // Check that pin is in range.
    if (kPin >= gAiCnt)
    {
        return E_AIO_PIN;
    }

    // Mode constants in FPGA API.
    static constexpr I8 modeRse = 1;
    static constexpr I8 modeDiff = 0;

    // Check that mode is valid.
    if ((kMode != modeRse) && (kMode != modeDiff))
    {
        return E_AIO_MODE;
    }

    // Set pin mode.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat,
                       NiFpga_WriteU8(mSession, gAiModeIds[kPin], kMode));
    if (stat != NiFpga_Status_Success)
    {
        return E_NI_FPGA_AIMODE;
    }

    return SUCCESS;
}

Result AnalogIo::setRange(const U32 kPin, const I8 kRange)
{
    // Check that AIO is initialized.
    if (!mInit)
    {
        return E_AIO_UNINIT;
    }

    // Check that pin is in range.
    if (kPin >= gAiCnt)
    {
        return E_AIO_PIN;
    }

    // Range constants in FPGA API.
    static constexpr U8 range1V = 3;
    static constexpr U8 range2V = 2;
    static constexpr U8 range5V = 1;
    static constexpr U8 range10V = 0;

    // Map range onto FPGA API constant.
    U8 rangeVal = 0;
    switch (kRange)
    {
        case 1:
            rangeVal = range1V;
            break;

        case 2:
            rangeVal = range2V;
            break;

        case 5:
            rangeVal = range5V;
            break;

        case 10:
            rangeVal = range10V;
            break;

        default:
            return E_AIO_RANGE;
    }

    // Set pin range.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat,
                       NiFpga_WriteU8(mSession, gAiRangeIds[kPin], rangeVal));
    if (stat != NiFpga_Status_Success)
    {
        return E_NI_FPGA_AIMODE;
    }

    return SUCCESS;
}

Result AnalogIo::read(const U32 kPin, F32& kVal)
{
    // Check that AIO is initialized.
    if (!mInit)
    {
        return E_AIO_UNINIT;
    }

    // Check that pin is in range.
    if (kPin >= gAiCnt)
    {
        return E_AIO_PIN;
    }

    // Read fixed point.
    NiFpga_Status stat = NiFpga_Status_Success;
    U32 fxp;
    NiFpga_MergeStatus(&stat,
                       NiFpga_ReadU32(mSession,
                                      gAiFxpResourceIds[kPin],
                                      &fxp));
    if (stat != NiFpga_Status_Success)
    {
        return E_AIO_READ;
    }

    // Convert fixed point to volts.
    kVal = NiFpga_ConvertFromFxpToFloat(gAiFxpTypeInfoIds[kPin], fxp);

    return SUCCESS;
}

Result AnalogIo::write(const U32 kPin, const F32 kVal)
{
    // Check that AIO is initialized.
    if (!mInit)
    {
        return E_AIO_UNINIT;
    }

    // Check that pin is in range.
    if (kPin >= gAoCnt)
    {
        return E_AIO_PIN;
    }

    // Check that output value is in range.
    static const F32 maxV = 10.0f;
    if ((kVal < -maxV) || (kVal > maxV))
    {
        return E_AIO_OUT;
    }

    // Convert volts to fixed point.
    const U32 fxp = NiFpga_ConvertFromFloatToFxp(gAoFxpTypeInfoIds[kPin], kVal);

    // Write fixed point to pin.
    NiFpga_Status stat = NiFpga_Status_Success;
    NiFpga_MergeStatus(&stat,
                       NiFpga_WriteU32(mSession, gAoFxpResourceIds[kPin], fxp));
    if (stat != NiFpga_Status_Success)
    {
        return E_AIO_READ;
    }

    return SUCCESS;
}

Result AnalogIo::release()
{
    // Check that AIO is initialized.
    if (!mInit)
    {
        return E_AIO_UNINIT;
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
