/*
 * Kneron Host Communication driver
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */
#ifndef KCOMM_COM_H
#define KCOMM_COM_H
#include "board_kl520.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM
#include "com.h"
// error code for cmd parser
#define NO_ERROR  0
#define NO_MATCH  1

#define RSP_NOT_IMPLEMENTED 0xFFFE
#define RSP_UKNOWN_CMD      0xFFFF
#define BAD_CONFIRMATION    2  // CMD_RESET RESPONSE
#define BAD_MODE            1
#define FILE_ERROR          1  // File data transfer error

typedef int (*CmdHandler)(MsgHdr*, CmdPram*, RspPram*, void*);

void kcomm_init(void);
void kcomm_reg_cmd_handler(CmdHandler handler);

/* cmd_handlers:
 * Trying to categorize group of cmds here.
 * Some of category of cmds are still coupled and should be fix in the future:
 * e.g.
 *   - ota_cmd uses CMD_RESET (base_cmds) to reset partition
 *   - dme_cmd uses CMD_NACK (base_cmds) to retrieve results */
extern void kcomm_enable_base_cmds(void);
extern void kcomm_enable_ota_cmds(void);
extern void kcomm_enable_dme_cmds(void);
extern void kcomm_enable_fid_cmds(void);
extern void kcomm_enable_test_cmds(void);
extern void kcomm_enable_camera_cmds(void);
extern void kcomm_enable_lwcom_cmds(void);
#endif
#endif
