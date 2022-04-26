/*
 * Generated with the FPGA Interface C API Generator 19.0
 * for NI-RIO 19.0 or later.
 */
#ifndef __NiFpga_IO_h__
#define __NiFpga_IO_h__

#ifndef NiFpga_Version
   #define NiFpga_Version 190
#endif

#include "NiFpga.h"

/**
 * The filename of the FPGA bitfile.
 *
 * This is a #define to allow for string literal concatenation. For example:
 *
 *    static const char* const Bitfile = "C:\\" NiFpga_IO_Bitfile;
 */
#define NiFpga_IO_Bitfile "NiFpga_IO.lvbitx"

/**
 * The signature of the FPGA bitfile.
 */
static const char* const NiFpga_IO_Signature = "B95157825AB2E6B99D4FFFC81B7AD39A";

#if NiFpga_Cpp
extern "C"
{
#endif

typedef enum
{
   NiFpga_IO_IndicatorBool_inDIO0 = 0x181E6,
   NiFpga_IO_IndicatorBool_inDIO1 = 0x181F2,
   NiFpga_IO_IndicatorBool_inDIO10 = 0x1803E,
   NiFpga_IO_IndicatorBool_inDIO11 = 0x1804A,
   NiFpga_IO_IndicatorBool_inDIO12 = 0x18056,
   NiFpga_IO_IndicatorBool_inDIO13 = 0x18062,
   NiFpga_IO_IndicatorBool_inDIO14 = 0x1806E,
   NiFpga_IO_IndicatorBool_inDIO15 = 0x1807A,
   NiFpga_IO_IndicatorBool_inDIO16 = 0x18086,
   NiFpga_IO_IndicatorBool_inDIO17 = 0x18092,
   NiFpga_IO_IndicatorBool_inDIO18 = 0x1809E,
   NiFpga_IO_IndicatorBool_inDIO19 = 0x180AA,
   NiFpga_IO_IndicatorBool_inDIO2 = 0x181FE,
   NiFpga_IO_IndicatorBool_inDIO20 = 0x180B6,
   NiFpga_IO_IndicatorBool_inDIO21 = 0x180C2,
   NiFpga_IO_IndicatorBool_inDIO22 = 0x180CE,
   NiFpga_IO_IndicatorBool_inDIO23 = 0x180DA,
   NiFpga_IO_IndicatorBool_inDIO24 = 0x180E6,
   NiFpga_IO_IndicatorBool_inDIO25 = 0x180F2,
   NiFpga_IO_IndicatorBool_inDIO26 = 0x180FE,
   NiFpga_IO_IndicatorBool_inDIO27 = 0x1810A,
   NiFpga_IO_IndicatorBool_inDIO3 = 0x1820A,
   NiFpga_IO_IndicatorBool_inDIO4 = 0x18216,
   NiFpga_IO_IndicatorBool_inDIO5 = 0x18002,
   NiFpga_IO_IndicatorBool_inDIO6 = 0x1800E,
   NiFpga_IO_IndicatorBool_inDIO7 = 0x1801A,
   NiFpga_IO_IndicatorBool_inDIO8 = 0x18026,
   NiFpga_IO_IndicatorBool_inDIO9 = 0x18032,
} NiFpga_IO_IndicatorBool;

typedef enum
{
   NiFpga_IO_ControlBool_outDIO0 = 0x181EE,
   NiFpga_IO_ControlBool_outDIO1 = 0x181FA,
   NiFpga_IO_ControlBool_outDIO10 = 0x18046,
   NiFpga_IO_ControlBool_outDIO11 = 0x18052,
   NiFpga_IO_ControlBool_outDIO12 = 0x1805E,
   NiFpga_IO_ControlBool_outDIO13 = 0x1806A,
   NiFpga_IO_ControlBool_outDIO14 = 0x18076,
   NiFpga_IO_ControlBool_outDIO15 = 0x18082,
   NiFpga_IO_ControlBool_outDIO16 = 0x1808E,
   NiFpga_IO_ControlBool_outDIO17 = 0x1809A,
   NiFpga_IO_ControlBool_outDIO18 = 0x180A6,
   NiFpga_IO_ControlBool_outDIO19 = 0x180B2,
   NiFpga_IO_ControlBool_outDIO2 = 0x18206,
   NiFpga_IO_ControlBool_outDIO20 = 0x180BE,
   NiFpga_IO_ControlBool_outDIO21 = 0x180CA,
   NiFpga_IO_ControlBool_outDIO22 = 0x180D6,
   NiFpga_IO_ControlBool_outDIO23 = 0x180E2,
   NiFpga_IO_ControlBool_outDIO24 = 0x180EE,
   NiFpga_IO_ControlBool_outDIO25 = 0x180FA,
   NiFpga_IO_ControlBool_outDIO26 = 0x18106,
   NiFpga_IO_ControlBool_outDIO27 = 0x18112,
   NiFpga_IO_ControlBool_outDIO3 = 0x18212,
   NiFpga_IO_ControlBool_outDIO4 = 0x1821E,
   NiFpga_IO_ControlBool_outDIO5 = 0x1800A,
   NiFpga_IO_ControlBool_outDIO6 = 0x18016,
   NiFpga_IO_ControlBool_outDIO7 = 0x18022,
   NiFpga_IO_ControlBool_outDIO8 = 0x1802E,
   NiFpga_IO_ControlBool_outDIO9 = 0x1803A,
   NiFpga_IO_ControlBool_outputEnableDIO0 = 0x181EA,
   NiFpga_IO_ControlBool_outputEnableDIO1 = 0x181F6,
   NiFpga_IO_ControlBool_outputEnableDIO10 = 0x18042,
   NiFpga_IO_ControlBool_outputEnableDIO11 = 0x1804E,
   NiFpga_IO_ControlBool_outputEnableDIO12 = 0x1805A,
   NiFpga_IO_ControlBool_outputEnableDIO13 = 0x18066,
   NiFpga_IO_ControlBool_outputEnableDIO14 = 0x18072,
   NiFpga_IO_ControlBool_outputEnableDIO15 = 0x1807E,
   NiFpga_IO_ControlBool_outputEnableDIO16 = 0x1808A,
   NiFpga_IO_ControlBool_outputEnableDIO17 = 0x18096,
   NiFpga_IO_ControlBool_outputEnableDIO18 = 0x180A2,
   NiFpga_IO_ControlBool_outputEnableDIO19 = 0x180AE,
   NiFpga_IO_ControlBool_outputEnableDIO2 = 0x18202,
   NiFpga_IO_ControlBool_outputEnableDIO20 = 0x180BA,
   NiFpga_IO_ControlBool_outputEnableDIO21 = 0x180C6,
   NiFpga_IO_ControlBool_outputEnableDIO22 = 0x180D2,
   NiFpga_IO_ControlBool_outputEnableDIO23 = 0x180DE,
   NiFpga_IO_ControlBool_outputEnableDIO24 = 0x180EA,
   NiFpga_IO_ControlBool_outputEnableDIO25 = 0x180F6,
   NiFpga_IO_ControlBool_outputEnableDIO26 = 0x18102,
   NiFpga_IO_ControlBool_outputEnableDIO27 = 0x1810E,
   NiFpga_IO_ControlBool_outputEnableDIO3 = 0x1820E,
   NiFpga_IO_ControlBool_outputEnableDIO4 = 0x1821A,
   NiFpga_IO_ControlBool_outputEnableDIO5 = 0x18006,
   NiFpga_IO_ControlBool_outputEnableDIO6 = 0x18012,
   NiFpga_IO_ControlBool_outputEnableDIO7 = 0x1801E,
   NiFpga_IO_ControlBool_outputEnableDIO8 = 0x1802A,
   NiFpga_IO_ControlBool_outputEnableDIO9 = 0x18036,
} NiFpga_IO_ControlBool;

typedef enum
{
   NiFpga_IO_ControlU8_modeAI0 = 0x1811A,
   NiFpga_IO_ControlU8_modeAI1 = 0x18126,
   NiFpga_IO_ControlU8_modeAI10 = 0x18192,
   NiFpga_IO_ControlU8_modeAI11 = 0x1819E,
   NiFpga_IO_ControlU8_modeAI12 = 0x181AA,
   NiFpga_IO_ControlU8_modeAI13 = 0x181B6,
   NiFpga_IO_ControlU8_modeAI14 = 0x181C2,
   NiFpga_IO_ControlU8_modeAI15 = 0x181CE,
   NiFpga_IO_ControlU8_modeAI2 = 0x18132,
   NiFpga_IO_ControlU8_modeAI3 = 0x1813E,
   NiFpga_IO_ControlU8_modeAI4 = 0x1814A,
   NiFpga_IO_ControlU8_modeAI5 = 0x18156,
   NiFpga_IO_ControlU8_modeAI6 = 0x18162,
   NiFpga_IO_ControlU8_modeAI7 = 0x1816E,
   NiFpga_IO_ControlU8_modeAI8 = 0x1817A,
   NiFpga_IO_ControlU8_modeAI9 = 0x18186,
   NiFpga_IO_ControlU8_rangeAI0 = 0x1811E,
   NiFpga_IO_ControlU8_rangeAI1 = 0x1812A,
   NiFpga_IO_ControlU8_rangeAI10 = 0x18196,
   NiFpga_IO_ControlU8_rangeAI11 = 0x181A2,
   NiFpga_IO_ControlU8_rangeAI12 = 0x181AE,
   NiFpga_IO_ControlU8_rangeAI13 = 0x181BA,
   NiFpga_IO_ControlU8_rangeAI14 = 0x181C6,
   NiFpga_IO_ControlU8_rangeAI15 = 0x181D2,
   NiFpga_IO_ControlU8_rangeAI2 = 0x18136,
   NiFpga_IO_ControlU8_rangeAI3 = 0x18142,
   NiFpga_IO_ControlU8_rangeAI4 = 0x1814E,
   NiFpga_IO_ControlU8_rangeAI5 = 0x1815A,
   NiFpga_IO_ControlU8_rangeAI6 = 0x18166,
   NiFpga_IO_ControlU8_rangeAI7 = 0x18172,
   NiFpga_IO_ControlU8_rangeAI8 = 0x1817E,
   NiFpga_IO_ControlU8_rangeAI9 = 0x1818A,
} NiFpga_IO_ControlU8;

#if !NiFpga_VxWorks

/* Indicator: inputAI0 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI0_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI0 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI0_Resource = 0x18114;


/* Indicator: inputAI1 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI1_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI1 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI1_Resource = 0x18120;


/* Indicator: inputAI10 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI10_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI10 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI10_Resource = 0x1818C;


/* Indicator: inputAI11 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI11_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI11 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI11_Resource = 0x18198;


/* Indicator: inputAI12 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI12_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI12 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI12_Resource = 0x181A4;


/* Indicator: inputAI13 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI13_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI13 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI13_Resource = 0x181B0;


/* Indicator: inputAI14 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI14_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI14 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI14_Resource = 0x181BC;


/* Indicator: inputAI15 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI15_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI15 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI15_Resource = 0x181C8;


/* Indicator: inputAI2 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI2_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI2 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI2_Resource = 0x1812C;


/* Indicator: inputAI3 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI3_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI3 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI3_Resource = 0x18138;


/* Indicator: inputAI4 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI4_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI4 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI4_Resource = 0x18144;


/* Indicator: inputAI5 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI5_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI5 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI5_Resource = 0x18150;


/* Indicator: inputAI6 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI6_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI6 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI6_Resource = 0x1815C;


/* Indicator: inputAI7 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI7_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI7 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI7_Resource = 0x18168;


/* Indicator: inputAI8 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI8_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI8 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI8_Resource = 0x18174;


/* Indicator: inputAI9 */
const NiFpga_FxpTypeInfo NiFpga_IO_IndicatorFxp_inputAI9_TypeInfo =
{
   1,
   24,
   5
};

/* Use NiFpga_ReadU32() to access inputAI9 */
const uint32_t NiFpga_IO_IndicatorFxp_inputAI9_Resource = 0x18180;


/* Control: outputAO0 */
const NiFpga_FxpTypeInfo NiFpga_IO_ControlFxp_outputAO0_TypeInfo =
{
   1,
   20,
   5
};

/* Use NiFpga_WriteU32() to access outputAO0 */
const uint32_t NiFpga_IO_ControlFxp_outputAO0_Resource = 0x181D4;


/* Control: outputAO1 */
const NiFpga_FxpTypeInfo NiFpga_IO_ControlFxp_outputAO1_TypeInfo =
{
   1,
   20,
   5
};

/* Use NiFpga_WriteU32() to access outputAO1 */
const uint32_t NiFpga_IO_ControlFxp_outputAO1_Resource = 0x181D8;


/* Control: outputAO2 */
const NiFpga_FxpTypeInfo NiFpga_IO_ControlFxp_outputAO2_TypeInfo =
{
   1,
   20,
   5
};

/* Use NiFpga_WriteU32() to access outputAO2 */
const uint32_t NiFpga_IO_ControlFxp_outputAO2_Resource = 0x181DC;


/* Control: outputAO3 */
const NiFpga_FxpTypeInfo NiFpga_IO_ControlFxp_outputAO3_TypeInfo =
{
   1,
   20,
   5
};

/* Use NiFpga_WriteU32() to access outputAO3 */
const uint32_t NiFpga_IO_ControlFxp_outputAO3_Resource = 0x181E0;


#endif /* !NiFpga_VxWorks */


#if NiFpga_Cpp
}
#endif

#endif
