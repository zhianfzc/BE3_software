/*
 * @name : v2k_api.c
 * @brief : Video capture interface for Mozart
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include <stdlib.h>
#include "types.h"
#include "framework/bitops.h"
#include "framework/utils.h"
#include "framework/v2k.h"
#include "framework/v2k_image.h"
#include "framework/framework_errno.h"
#include "dbg.h"


#define FRAME_SIZE_RGB(xres,yres,mbpp)  ((xres) * (yres) * (mbpp) / 8)

int calc_framesize(
        unsigned short frame_width, 
        unsigned short frame_height, 
        unsigned int frame_fmt)
{
    switch(frame_fmt)
    {
        case V2K_PIX_FMT_RGB565:
            return FRAME_SIZE_RGB(frame_width, frame_height, 16);

        case V2K_PIX_FMT_RAW8:
            return FRAME_SIZE_RGB(frame_width, frame_height, 8);
        
        default:
            break;
    }
    return 0;
}
