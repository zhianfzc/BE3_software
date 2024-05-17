/*
 * Kneron Application AgeGender APIs
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __KDP_APP_AGE_GENDER_H_
#define __KDP_APP_AGE_GENDER_H_

#include <stdint.h>
#include "kdp_app_dme.h"
#include "ipc.h"

uint32_t get_fd_age_gender_result(fd_age_gender_res *p_out_p, struct kdp_dme_cfg *dme_conf);

#endif /* __KDP_APP_AGE_GENDER_H_ */
