/*
 * @name : drivers.c
 * @brief : centralized driver management
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */
#include "board_kl520.h"
#include "drivers.h"
#include "kdp520_i2c.h"


int kdp_drv_init(void)
{
#if CFG_I2C_0_ENABLE == YES
    kdp_drv_i2c_init(I2C_ADAP_0, 200000, FALSE);
#endif    
#if CFG_I2C_1_ENABLE == YES
    kdp_drv_i2c_init(I2C_ADAP_1, 200000, FALSE);
#endif
#if CFG_I2C_2_ENABLE == YES    
    kdp_drv_i2c_init(I2C_ADAP_2, 200000, FALSE);
#endif
#if CFG_I2C_3_ENABLE == YES     
    kdp_drv_i2c_init(I2C_ADAP_3, 200000, FALSE);
#endif
    
    return 0;
}

