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

/////////////////////////////// ConfigTokenizer ////////////////////////////////

#define E_TOK_INVALID (32)

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

/////////////////////////////////// Socket /////////////////////////////////////

#define E_SOCK_PROTO (256)
#define E_SOCK_OPEN (257)
#define E_SOCK_BIND (258)
#define E_SOCK_SEND (259)
#define E_SOCK_RECV (260)
#define E_SOCK_SEL (261)
#define E_SOCK_CLOSE (262)
#define E_SOCK_NULL (263)
#define E_SOCK_SEL_NONE (264)

/////////////////////////////////// Thread /////////////////////////////////////

#define E_THR_UNINIT (288)
#define E_THR_POL (289)
#define E_THR_PRI (290)
#define E_THR_CREATE (291)
#define E_THR_AFF (292)
#define E_THR_EXIST (293)
#define E_THR_AWAIT (294)
#define E_THR_RANGE (295)
#define E_THR_NULL (296)

///////////////////////////////// Linux Thread /////////////////////////////////

#define E_THR_INIT_ATTR (1024)
#define E_THR_DTRY_ATTR (1025)
#define E_THR_INH_PRI (1025)

////////////////////////////////// Spinlock ////////////////////////////////////

#define E_SLK_CREATE (1056)
#define E_SLK_UNINIT (1057)
#define E_SLK_ACQ (1058)
#define E_SLK_REL (1059)

#endif
