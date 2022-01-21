#ifndef SFA_RESULT_HPP
#define SFA_RESULT_HPP

#include "sfa/BasicTypes.hpp"

typedef I32 Result;

#define SUCCESS (0)
#define E_FILE (1)
#define E_EMPTY (2)
#define E_TYPE (3)
#define E_NULLPTR (4)
#define E_TOKENIZE (5)
#define E_UNREACHABLE (6)
#define E_PARSE (7)
#define E_KEY (8)
#define E_SIZE (9)
#define E_STATE (10)
#define E_ENUM (11)
#define E_DUPLICATE (12)
#define E_RANGE (13)
#define E_RESERVED (14)
#define E_UNINITIALIZED (15)
#define E_TIME (16)
#define E_TRANSITION (17)
#define E_LAYOUT (18)
#define E_REINIT (19)

////////////////////////////// StateVectorParser ///////////////////////////////

#define E_SVP_RGN_DUPE (64)
#define E_SVP_RGN_NAME (65)
#define E_SVP_RGN_EMPTY (66)
#define E_SVP_NO_RGNS (67)
#define E_SVP_ELEM_DUPE (68)
#define E_SVP_ELEM_NAME (69)
#define E_SVP_ELEM_TYPE (70)
#define E_SVP_SEC_NAME (71)
#define E_SVP_TOK (72)
#define E_SVP_RGN_TOK (73)

#endif
