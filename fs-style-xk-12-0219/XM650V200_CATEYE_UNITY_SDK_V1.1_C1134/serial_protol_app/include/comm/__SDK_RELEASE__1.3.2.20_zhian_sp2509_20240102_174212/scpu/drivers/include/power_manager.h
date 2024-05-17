/*
 * Kneron Power Manager driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __POWER_MANAGER_H__
#define __POWER_MANAGER_H__

#include "base.h"

/* SCU_REG_INT_EN & SCU_REG_INT_STS */
#define SCU_INT_RTC_PERIODIC        BIT17
#define SCU_INT_RTC_ALARM           BIT16
#define SCU_INT_PLL_UPDATE          BIT8
#define SCU_INT_FCS                 BIT6
#define SCU_INT_BUSSPEED            BIT5
#define SCU_INT_WAKEUP              BIT3
#define SCU_INT_PWRBTN_RISE         BIT1
#define SCU_INT_PWRBTN_FALL         BIT0

enum pm_device_id {
    PM_DEVICE_NONE = 0,
    PM_DEVICE_CAMERA1,
    PM_DEVICE_CAMERA2,
    PM_DEVICE_DISPLAY1,
    PM_DEVICE_DISPLAY2,
    PM_DEVICE_HOST_COM,
    PM_DEVICE_OTA_UPDATE,
    PM_DEVICE_NCPU_INFERENCE,
    PM_DEVICE_UNUSED4,
    PM_DEVICE_UNUSED5,
    PM_DEVICE_UNUSED6,
    PM_DEVICE_UNUSED7,
    PM_DEVICE_UNUSED8,
    PM_DEVICE_MAX,
};

/* Prototypes for callback functions */
typedef int (*pm_call)(enum pm_device_id dev_id);

struct pm_s {
    pm_call     nap;
    pm_call     wakeup_nap;
    pm_call     deep_nap;
    pm_call     wakeup_deep_nap;
    pm_call     sleep;
    pm_call     wakeup_sleep;
    pm_call     deep_sleep;
    pm_call     wakeup_deep_sleep;
};

/* PM APIs */
__NO_RETURN void power_manager_cpu_idle(void);
void power_manager_init(void);
void power_manager_error_notify(uint32_t code, void *object_id);
void power_manager_reset(void);
void power_manager_sleep(void);
void power_manager_deep_sleep(void);
void power_manager_shutdown(void);

/* Registration APIs */
int power_manager_register(enum pm_device_id dev_id, struct pm_s *pm_p);
void power_manager_unregister(enum pm_device_id dev_id, struct pm_s *pm_p);

#endif
