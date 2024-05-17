/********************************************************************
 * Copyright (c) 2020 Kneron, Inc. All Rights Reserved.
 *
 * The information contained herein is property of Kneron, Inc.
 * Terms and conditions of usage are described in detail in Kneron
 * STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information.
 * NO WARRANTY of ANY KIND is provided. This heading must NOT be removed
 * from the file.
 ********************************************************************/

/**@addtogroup  KDP_COM
 * @{
 * @brief       Kneron NPU driver
 * @copyright   Copyright (C) 2020 Kneron, Inc. All rights reserved.
 */

#ifndef KDP_COM_H
#define KDP_COM_H

#include "cmsis_os2.h"
#include "types.h"
#include "ipc.h"

typedef void (*ipc_handler_t)(int ipc_idx, int state);

#define CMD_FLAGS(i)                (kdp_com_get_output_ptr()->cmd_flags[i])
#define CMD_FLAGS_IS_ACTIVE(i)      (kdp_com_get_output_ptr()->cmd_flags[i] == IMAGE_STATE_ACTIVE)
#define CMD_FLAGS_IS_INACTIVE(i)    (kdp_com_get_output_ptr()->cmd_flags[i] == IMAGE_STATE_INACTIVE)
#define CMD_STATUS(i)               (kdp_com_get_input_ptr()->cmd_status[i])
#define CMD_STATUS_IS_NPU_BUSY(i)   (kdp_com_get_input_ptr()->cmd_status[i] == IMAGE_STATE_NPU_BUSY)
#define CMD_STATUS_IS_NPU_DONE(i)   (kdp_com_get_input_ptr()->cmd_status[i] == IMAGE_STATE_NPU_DONE)
#define CMD_STATUS_IS_PROC_DONE(i)  (kdp_com_get_input_ptr()->cmd_status[i] == IMAGE_STATE_POST_PROCESSING_DONE)


/**
 * @brief Initialize NPU functionality
 * @param ipc_handler IPC callback
 */
void kdp_com_init(BOOL log_enable, ipc_handler_t ipc_handler);
/**
 * @brief Set model information
 * @param model_info_addr model information address
 * @param info_idx information index
 * @param slot_idx slot index
 */
void kdp_com_set_model(struct kdp_model_s *model_info_addr, uint32_t info_idx, int32_t slot_idx, uint32_t model_type);

/**
 * @brief Get available COM
 * @return COM index
 */
int kdp_com_get_avail_com(void);

/**
 * @brief Set active model index
 * @param index model slot index
 * @return available COM
 */
int kdp_com_set_model_active(uint32_t index);

/**
 * @brief Get active model index
 * @return index
 */
int kdp_com_get_model_active(void);

/**
 * @brief Set active image index
 * @param index image index
 * @return available COM
 */
int kdp_com_set_image_active(uint32_t index);

/**
 * @brief Get active image index
 * @return index
 */
int kdp_com_get_image_active(void);

/**
 * @brief Set SCPU debug level
 * @param lvl level
 */
void kdp_com_set_scpu_debug_lvl(uint32_t lvl);

/**
 * @brief Set NCPU debug level
 * @param lvl level
 */
void kdp_com_set_ncpu_debug_lvl(uint32_t lvl);

/**
 * @brief Trigger NCPU interrupt
 */
void kdp_com_ncpu_trigger_int(uint32_t model_type);

/**
 * @brief Get ncpu_to_scpu_s
 * @return IPC struct
 */
struct ncpu_to_scpu_s* kdp_com_get_input_ptr(void);

/**
 * @brief Get scpu_to_ncpu_s
 * @return IPC struct
 */
struct scpu_to_ncpu_s* kdp_com_get_output_ptr(void);

BOOL kdp_com_is_inited(void);

#endif
