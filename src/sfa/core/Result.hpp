#ifndef SFA_RESULT_HPP
#define SFA_RESULT_HPP

#include "sfa/core/BasicTypes.hpp"

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

    // Task
    E_TSK_MODE = 128,
    E_TSK_UNINIT = 129,

    // StateMachine
    E_SM_UNINIT = 160,
    E_SM_REINIT = 161,
    E_SM_STATE = 162,
    E_SM_NULL = 163,
    E_SM_TIME = 164,
    E_SM_TRANS = 165,
    E_SM_EXIT = 166,
    E_SM_EMPTY = 167,

////////////////////////// Support Library Error Codes /////////////////////////

    // ConfigTokenizer
    E_TOK_INVALID = 256,
    E_TOK_FILE = 257,

    // StateVectorParser
    E_SVP_RGN_DUPE = 288,
    E_SVP_RGN_NAME = 289,
    E_SVP_RGN_EMPTY = 290,
    E_SVP_NO_RGNS = 291,
    E_SVP_ELEM_DUPE = 292,
    E_SVP_ELEM_NAME = 293,
    E_SVP_ELEM_TYPE = 294,
    E_SVP_SEC_NAME = 295,
    E_SVP_TOK = 296,
    E_SVP_RGN_TOK = 297,
    E_SVP_FILE = 298,

    // StateMachineParser
    E_SMP_SEC = 320,
    E_SMP_MULT_LOC = 321,
    E_SMP_LOC_NAME = 322,
    E_SMP_LOC_TYPE = 323,
    E_SMP_LOC_RSVD = 324,
    E_SMP_LOC_DUPE = 325,
    E_SMP_LOC_OP = 326,
    E_SMP_LOC_ANN = 327,
    E_SMP_LOC_VAL = 328,
    E_SMP_RO_RED = 328,

/////////////////////////////// PSL Error Codes ////////////////////////////////

    // Socket
    E_SOK_PROTO = 512,
    E_SOK_OPEN = 513,
    E_SOK_BIND = 514,
    E_SOK_SEND = 515,
    E_SOK_RECV = 516,
    E_SOK_SEL = 517,
    E_SOK_CLOSE = 518,
    E_SOK_NULL = 519,
    E_SOK_SEL_NONE = 520,
    E_SOK_UNINIT = 521,
    E_SOK_REINIT = 522,

    // Thread
    E_THR_UNINIT = 544,
    E_THR_POL = 545,
    E_THR_PRI = 546,
    E_THR_CREATE = 547,
    E_THR_AFF = 548,
    E_THR_EXIST = 549,
    E_THR_AWAIT = 550,
    E_THR_RANGE = 551,
    E_THR_NULL = 552,
    E_THR_REINIT = 553,
    E_THR_INIT_ATTR = 554,
    E_THR_DTRY_ATTR = 555,
    E_THR_INH_PRI = 556,

    // Spinlock
    E_SLK_CREATE = 576,
    E_SLK_UNINIT = 577,
    E_SLK_ACQ = 578,
    E_SLK_REL = 579,

    // DigitalIo
    E_DIO_UNINIT = 608,
    E_DIO_REINIT = 609,
    E_DIO_PIN = 610,
    E_DIO_MODE = 611,

    // AnalogIo
    E_AIO_UNINIT = 640,
    E_AIO_REINIT = 641,
    E_AIO_PIN = 642,
    E_AIO_RANGE = 643,
};

#endif
