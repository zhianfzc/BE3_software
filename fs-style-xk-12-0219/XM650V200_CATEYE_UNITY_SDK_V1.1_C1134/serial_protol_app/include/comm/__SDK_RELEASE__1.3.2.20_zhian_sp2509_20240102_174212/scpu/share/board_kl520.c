/*
 * @name : board_kdp520.c
 * @brief : Setup code for mozart/Haps
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#include "board_kl520.h"
#include "board_ddr_table.h"
#include "kneron_mozart.h"
#include "framework/init.h"
#include "framework/framework_driver.h"
#include "framework/ioport.h"
#include "framework/v2k_image.h"
#include "media/camera/sensor.h"


#if CFG_I2C_0_ENABLE == YES

struct ioport_setting INITDATA _io_i2c_0[] = {
    {
        .start      = IIC_FTIIC010_0_PA_BASE,
        .flags      = IORESOURCE_MEM,
    }, {
        .start      = IIC_FTIIC010_0_IRQ,
        .flags      = IORESOURCE_IRQ,
    },
};
struct core_device INITDATA kdp520_i2c_0 = {
    .name           = "kdp520_i2c",
    .ioport         = _io_i2c_0,
    .uuid           = 0,
    .pin_ctx = {
        .platform_data = NULL, //&default_i2c_data,
    },
};
#endif
#if CFG_I2C_1_ENABLE == YES
struct ioport_setting INITDATA _io_i2c_1[] = {
    {
        .start      = IIC_FTIIC010_1_PA_BASE,
        .flags      = IORESOURCE_MEM,
    }, {
        .start      = IIC_FTIIC010_1_IRQ,
        .flags      = IORESOURCE_IRQ,
    },
};
struct core_device INITDATA kdp520_i2c_1 = {
    .name           = "kdp520_i2c",
    .ioport         = _io_i2c_1,
    .uuid           = 1,
    .pin_ctx = {
        .platform_data = NULL, //&default_i2c_data,
    },
};
#endif
#if CFG_I2C_2_ENABLE == YES
struct ioport_setting INITDATA _io_i2c_2[] = {
    {
        .start      = IIC_FTIIC010_2_PA_BASE,
        .flags      = IORESOURCE_MEM,
    }, {
        .start      = IIC_FTIIC010_2_IRQ,
        .flags      = IORESOURCE_IRQ,
    },
};
struct core_device INITDATA kdp520_i2c_2 = {
    .name           = "kdp520_i2c",
    .ioport         = _io_i2c_2,
    .uuid           = 2,
    .pin_ctx = {
        .platform_data = NULL, //&default_i2c_data,
    },
};
#endif
#if CFG_I2C_3_ENABLE == YES
struct ioport_setting INITDATA io_i2c3[] = {
    {
        .start      = IIC_FTIIC010_3_PA_BASE,
        .flags      = IORESOURCE_MEM,
    }, {
        .start      = IIC_FTIIC010_3_IRQ,
        .flags      = IORESOURCE_IRQ,
    },
};
struct core_device INITDATA kdp520_i2c_3 = {
    .name           = "kdp520_i2c",
    .ioport         = io_i2c3,
    .uuid           = 3,
    .pin_ctx = {
        .platform_data = NULL, //&default_i2c_data,
    },    
};
#endif
struct core_device kdp520_touch = {
    .name           = "kdp520_touch",
};
