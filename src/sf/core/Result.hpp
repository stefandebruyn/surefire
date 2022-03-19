#ifndef SF_RESULT_HPP
#define SF_RESULT_HPP

#include "sf/core/BasicTypes.hpp"

typedef I32 Result;

enum : Result
{
    SUCCESS = 0,

/////////////////////////// Core Library Error Codes ///////////////////////////

    // Region
    E_RGN_SIZE = 32,

    // StateVector
    E_SV_UNINIT = 64,
    E_SV_REINIT = 65,
    E_SV_EMPTY = 66,
    E_SV_TYPE = 67,
    E_SV_NULL = 68,
    E_SV_KEY = 69,
    E_SV_LAYOUT = 70,
    E_SV_ELEM_DUPE = 71,
    E_SV_RGN_DUPE = 72,

    // Task
    E_TSK_UNINIT = 128,
    E_TSK_REINIT = 129,
    E_TSK_MODE = 130,

    // StateMachine
    E_SM_UNINIT = 160,
    E_SM_REINIT = 161,
    E_SM_STATE = 162,
    E_SM_NULL = 163,
    E_SM_TIME = 164,
    E_SM_TRANS = 165,
    E_SM_TR_EXIT = 166,
    E_SM_EMPTY = 167,

/////////////////////////// Config Library Error Codes /////////////////////////

    // Tokenizer
    E_TOK_INVALID = 256,
    E_TOK_FILE = 257,

    // StateVectorParser
    E_SVP_ELEM_NAME = 288,
    E_SVP_ELEM_TYPE = 289,
    E_SVP_RGN = 290,

    // StateMachineParser
    E_SMP_SEC = 320,
    E_SMP_ELEM_TYPE = 321,
    E_SMP_ELEM_NAME = 322,
    E_SMP_NAME_RSVD = 324,
    E_SMP_NAME_DUPE = 325,
    E_SMP_ANNOT = 325,
    E_SMP_RO_MULT = 326,
    E_SMP_AL_MULT = 327,
    E_SMP_LOC_MULT = 328,
    E_SMP_LOC_OP = 329,
    E_SMP_LOC_VAL = 330,
    E_SMP_SV_MULT = 331,
    E_SMP_SV_NAME = 332,
    E_SMP_SV_TYPE = 333,
    E_SMP_EOF = 334,
    E_SMP_LAB = 335,
    E_SMP_GUARD = 336,
    E_SMP_BRACE = 337,
    E_SMP_ELSE = 338,
    E_SMP_ACT_TOK = 339,
    E_SMP_ACT_ELEM = 340,
    E_SMP_ACT_OP = 341,
    E_SMP_ACT_EXPR = 342,
    E_SMP_TR_OP = 343,
    E_SMP_TR_DEST = 344,
    E_SMP_TR_TOK = 345,
    E_SMP_TR_JUNK = 346,
    E_SMP_TOK = 347,

    // ExpressionParser
    E_EXP_OP = 384,
    E_EXP_SYNTAX = 385,
    E_EXP_TOK = 386,
    E_EXP_EMPTY = 387,
    E_EXP_PAREN = 388,

    // StateVectorCompiler
    E_SVC_FILE = 416,
    E_SVC_RGN_EMPTY = 417,
    E_SVC_ELEM_TYPE = 418,
    E_SVC_RGN_DUPE = 419,
    E_SVC_ELEM_DUPE = 420,

    // StateMachineCompiler
    E_SMC_FILE = 448,
    E_SMC_SV_ELEM = 449,
    E_SMC_TYPE = 450,
    E_SMC_TYPE_MISM = 451,
    E_SMC_G_TYPE = 452,
    E_SMC_S_TYPE = 453,
    E_SMC_NO_G = 454,
    E_SMC_NO_S = 455,
    E_SMC_ELEM_DUPE = 456,
    E_SMC_ASG_ELEM = 457,
    E_SMC_STATE = 458,
    E_SMC_ELEM_RO = 459,
    E_SMC_TR_EXIT = 460,

    // ExpressionCompiler
    E_EXC_NULL = 480,
    E_EXC_NUM = 481,
    E_EXC_TYPE = 482,
    E_EXC_ELEM = 483,
    E_EXC_OVFL = 484,
    E_EXC_ARITY = 485,
    E_EXC_FUNC = 486,
    E_EXC_WIN = 487,

    // StateScriptParser
    E_SSP_SEC = 512,

/////////////////////////////// PSL Error Codes ////////////////////////////////

    // Socket
    E_SOK_UNINIT = 1024,
    E_SOK_REINIT = 1025,
    E_SOK_PROTO = 1026,
    E_SOK_OPEN = 1027,
    E_SOK_BIND = 1028,
    E_SOK_SEND = 1029,
    E_SOK_RECV = 1030,
    E_SOK_SEL = 1031,
    E_SOK_CLOSE = 1032,
    E_SOK_NULL = 1033,
    E_SOK_SEL_NONE = 1034,

    // Thread
    E_THR_UNINIT = 1056,
    E_THR_REINIT = 1057,
    E_THR_POL = 1058,
    E_THR_PRI = 1059,
    E_THR_CREATE = 1060,
    E_THR_AFF = 1061,
    E_THR_EXIST = 1062,
    E_THR_AWAIT = 1063,
    E_THR_RANGE = 1064,
    E_THR_NULL = 1065,
    E_THR_INIT_ATTR = 1066,
    E_THR_DTRY_ATTR = 1067,
    E_THR_INH_PRI = 1068,

    // Spinlock
    E_SLK_UNINIT = 1088,
    E_SLK_REINIT = 1089,
    E_SLK_CREATE = 1090,
    E_SLK_ACQ = 1091,
    E_SLK_REL = 1092,

    // DigitalIO
    E_DIO_UNINIT = 1120,
    E_DIO_REINIT = 1121,
    E_DIO_PIN = 1122,
    E_DIO_MODE = 1123,

    // AnalogIO
    E_AIO_UNINIT = 1142,
    E_AIO_REINIT = 1143,
    E_AIO_PIN = 1144,
    E_AIO_RANGE = 1145,
};

#endif
