/*
 * Kneron memory APIs
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_MEMORY_H_
#define __KDP_MEMORY_H_

#include <stdint.h>

/**
 * @brief To initialize available DDR block 
 * @param start_addr the start address of DDR block
 * @param eed_addr the end address of DDR block
 */
void        kdp_ddr_init(uint32_t start_addr, uint32_t end_addr);

/**
 * @brief to reserve DDR memory
 * @param numbtye size in byte
 * @return the address of reserve block
 */
uint32_t    kdp_ddr_reserve(uint32_t numbyte);

/**
 * @brief to release allocated memory
 * @param addr DDR address
 */
void        kdp_ddr_free(uint32_t addr);

#endif

