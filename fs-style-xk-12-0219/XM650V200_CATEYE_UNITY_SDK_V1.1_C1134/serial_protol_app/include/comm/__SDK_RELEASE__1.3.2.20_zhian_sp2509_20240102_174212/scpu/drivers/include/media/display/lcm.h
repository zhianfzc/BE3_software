#ifndef __LCM_H__
#define __LCM_H__


#include "base.h"
#include "io.h"
#include "kneron_mozart.h"

#define LCM_BASE                                            SLCD_FTLCDC210_PA_BASE

#define LCM_REG_TIMING                                      (LCM_BASE + 0x200)
#define LCM_REG_RDY                                         (LCM_BASE + 0x204)
#define LCM_REG_RS                                          (LCM_BASE + 0x208)
#define LCM_REG_DATA                                        (LCM_BASE + 0x20C)
#define LCM_REG_CMD                                         (LCM_BASE + 0x210)
#define LCM_REG_OP_MODE                                     (LCM_BASE + 0x214)
#define LCM_REG_ENABLE                                      (LCM_BASE + 0x228)

#define LCM_REG_TIMING_P(base)                              (base + 0x200)
#define LCM_REG_RDY_P(base)                                 (base + 0x204)
#define LCM_REG_RS_P(base)                                  (base + 0x208)
#define LCM_REG_DATA_P(base)                                (base + 0x20C)
#define LCM_REG_CMD_P(base)                                 (base + 0x210)
#define LCM_REG_OP_MODE_P(base)                             (base + 0x214)
#define LCM_REG_ENABLE_P(base)                              (base + 0x228)


/* LCM Timing Control Register (Offset : 0x200) */
/* Tpwh width for the read cycles */
#define LCM_REG_TIMING_SET_Tpwh_r(val)                      SET_MASKED_BITS(LCM_REG_TIMING, val, 16, 19)
/* Tas width */
#define LCM_REG_TIMING_SET_Tas(val)                         SET_MASKED_BITS(LCM_REG_TIMING, val, 12, 15)
/* Tah width */
#define LCM_REG_TIMING_SET_Tah(val)                         SET_MASKED_BITS(LCM_REG_TIMING, val, 8, 11)
/* Tpwd width */
#define LCM_REG_TIMING_SET_Tpwl(val)                        SET_MASKED_BITS(LCM_REG_TIMING, val, 4, 7)
/* Tpwh width for write cycles */
#define LCM_REG_TIMING_SET_Tpwh_w(val)                      SET_MASKED_BITS(LCM_REG_TIMING, val, 0, 3)

/* LCM Ready Register (Offset : 0x204) */
#define LCM_REG_RDY_READY_FOR_ACCESS                        BIT0
#define LCM_REG_RDY_GET_Rdy_C_D                             GET_BIT(LCM_REG_RDY, 0)

/* LCM Data/Command Read Control Register, LCM_RS (Offset 0x208) */
/* write 1: issue the read data phase at RS=1 */
#define LCM_REG_RS_DMYRD_RS                                 BIT0
#define LCM_REG_RS_GET_DMYRD_RS()                           GET_BIT(LCM_REG_RS, 0)
#define LCM_REG_RS_SET_DMYRD_RS(val)                        SET_MASKED_BIT(LCM_REG_RS, val, 0)

/* LCM Operation Mode Select Register, LCM_OP_MODE (Offset 0x214) */
#define LCM_REG_OP_MODE_SET_16bit_mode(val)                 SET_MASKED_BITS(LCM_REG_OP_MODE, val, 6, 7)
#define LCM_REG_OP_MODE_SET_Bus_IF(val)                     SET_MASKED_BITS(LCM_REG_OP_MODE, val, 4, 5)
#define LCM_REG_OP_MODE_SET_Panel_IF(val)                   SET_MASKED_BITS(LCM_REG_OP_MODE, val, 2, 3)
#define LCM_REG_OP_MODE_SET_C68(val)                        SET_MASKED_BITS(LCM_REG_OP_MODE, val, 0, 1)

/* LCM Enable Register, LCM_ENABLE (Offset 0x228) */
#define LCM_REG_ENABLE_GET_BLCTRL_SET()                     GET_BIT(LCM_REG_ENABLE, 12)
#define LCM_REG_ENABLE_GET_RST_INV()                        GET_BIT(LCM_REG_ENABLE, 11)
#define LCM_REG_ENABLE_GET_RST_CLR()                        GET_BIT(LCM_REG_ENABLE, 9)
#define LCM_REG_ENABLE_GET_RST_SET()                        GET_BIT(LCM_REG_ENABLE, 8)
#define LCM_REG_ENABLE_GET_LCM_En()                         GET_BIT(LCM_REG_ENABLE, 0)

#define LCM_REG_ENABLE_SET_BLCTRL_CLR(val)                  SET_MASKED_BIT(LCM_REG_ENABLE, val, 13)
#define LCM_REG_ENABLE_SET_BLCTRL_SET(val)                  SET_MASKED_BIT(LCM_REG_ENABLE, val, 12)
#define LCM_REG_ENABLE_SET_RST_INV(val)                     SET_MASKED_BIT(LCM_REG_ENABLE, val, 11)
#define LCM_REG_ENABLE_SET_RST_CLR()                        SET_BIT(LCM_REG_ENABLE, 9)
//#define LCM_REG_ENABLE_SET_RST_CLR(val)                     SET_MASKED_BIT(LCM_REG_ENABLE, val, 9)
#define LCM_REG_ENABLE_SET_RST_SET()                        SET_BIT(LCM_REG_ENABLE, 8)
//#define LCM_REG_ENABLE_SET_RST_SET(val)                     //SET_MASKED_BIT(LCM_REG_ENABLE, val, 8)
#define LCM_REG_ENABLE_SET_LCM_En(val)                      SET_MASKED_BIT(LCM_REG_ENABLE, val, 0)

#endif
