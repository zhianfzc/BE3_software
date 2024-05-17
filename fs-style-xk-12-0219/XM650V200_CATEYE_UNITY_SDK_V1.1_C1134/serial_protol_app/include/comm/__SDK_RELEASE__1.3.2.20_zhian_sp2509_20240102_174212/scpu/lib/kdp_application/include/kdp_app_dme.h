/*
 * Kneron Application Dynamic Model Execution APIs
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_APP_DME_H_
#define __KDP_APP_DME_H_

#include <stdint.h>
#include "kdp_app.h"
#include "ipc.h"

struct kdp_img_conf_s {
    int32_t     image_col;        /// column size
    int32_t     image_row;        /// row size
    int32_t     image_ch;         /// channel size
    uint32_t    image_format;     /// image format. KDP_APP_IMAGE_FORMAT_XXXX
    uint32_t    image_mem_addr;   /// address of image data
};

struct kdp_dme_cfg {
    int32_t models_selection;
    int32_t output_num;
    struct kdp_img_conf_s img_conf;
};

struct kdp_img_cfg kdp_app_dme_gen_img_cfg(const struct kdp_dme_cfg *dme_conf, int img_buf_idx);

/**
 * @brief do dynamic model execution (DME) to get detection output or raw output
 * @param [out] p_out_p dynamic model execution result
 * @param [in] img_conf image configuration
 * @return OK or Err code
 */
int32_t kdp_app_dme(dme_res* p_out_p, struct kdp_dme_cfg *img_conf);

#define kapp_dme kdp_app_dme

#endif
