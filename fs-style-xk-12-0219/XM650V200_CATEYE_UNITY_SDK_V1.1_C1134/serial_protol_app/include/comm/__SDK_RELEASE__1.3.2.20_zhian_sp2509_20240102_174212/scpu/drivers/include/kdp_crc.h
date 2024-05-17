/**
 * @file kdp_crc.h
 * 
 * @brief Kneron crc driver header
 *
 * Copyright (C) 2018 Kneron, Inc. All rights reserved.
 *
 */
#ifndef __KDP_CRC16_H__
#define __KDP_CRC16_H__

#include "types.h"

#define CRC16_CONSTANT 0x8005

/**
 * @brief generate crc16 code
 * @parem [in] data
 * @parem [in] size : data size
 */
u16 kdp_gen_crc16(u8 *data, u32 size);
u32 kdp_gen_sha32(u8 *data, u32 size);
u32 kdp_gen_sum32(u8 *data, u32 size);

#endif
