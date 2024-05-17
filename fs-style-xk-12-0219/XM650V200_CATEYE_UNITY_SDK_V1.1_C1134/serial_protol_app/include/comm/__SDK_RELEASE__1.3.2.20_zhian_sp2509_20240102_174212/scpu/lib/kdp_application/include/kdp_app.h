/*
 * Kneron Application Shared Data
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

/**
 * @file kdp_app.h
 * @brief Public API header of kdp_application generic functions
 */

#ifndef __KDP_APP_H_
#define __KDP_APP_H_


#include "types.h"
#include "model_type.h"
#include "ipc.h"


enum kapp_status_code_e {
    KAPP_OK = 0,
    KAPP_UNKNOWN_ERR = 1,
};

/* no need to align with cam source index */
#define KAPP_IMG_SRC_RGB  0
#define KAPP_IMG_SRC_NIR  1
#define KAPP_IMG_SRC_LV   2
#define KAPP_IMG_SRC_MAX  3

/**
 * @brief to get channel size by image format
 *
 * @param [in] image format
 * @return channel size
 */
//uint32_t kdp_app_get_chnl_size(uint32_t format);

/**
 * @brief to get pixel size by image format
 *
 * @param [in] image format
 * @return pixel size
 */
uint32_t kdp_app_get_pixel_size(uint32_t image_format);

#endif
