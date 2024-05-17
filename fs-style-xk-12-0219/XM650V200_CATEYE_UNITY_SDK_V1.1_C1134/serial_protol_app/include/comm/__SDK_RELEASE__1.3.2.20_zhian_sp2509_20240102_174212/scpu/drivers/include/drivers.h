#ifndef __DRIVERS_H__
#define __DRIVERS_H__

//#define USE_KDRV
#ifdef USE_KDRV
#include "kdrv_adc.h"
#include "kdrv_mpu.h"
#include "kdrv_ddr.h"
#include "kdrv_clock.h"
#include "kdrv_power.h"
#include "kdrv_system.h"
#include "kdrv_uart.h"
#define kdp520_adc_init                 kdrv_adc_initialize
#define kdp520_adc_read                 kdrv_adc_read
#define mpu_config                      kdrv_mpu_config
#define mpu_niram_enable                kdrv_mpu_niram_enable
#define mpu_niram_disalbe               kdrv_mpu_niram_disable
#define ddr_init                        kdrv_ddr_system_init
#define ddr_self_refresh_enter          kdrv_ddr_self_refresh_enter
#define ddr_self_refresh_exit           kdrv_ddr_self_refresh_exit
#define ddr_wakeup                      kdrv_ddr_wakeup
#define clock_mgr_set_scuclkin          kdrv_clock_set_scuclkin
#define clock_mgr_init                  kdrv_clock_initialize
#define clk_enable                      kdrv_clock_enable
#define clk_disable                     kdrv_clock_disable
#define clock_mgr_change_pll3_clock     kdrv_clock_change_pll3_clock
#define clock_mgr_change_pll5_clock     kdrv_clock_change_pll5_clock
#define POWER_MGR_OPS_FCS               POWER_OPS_FCS
#define POWER_MGR_OPS_CHANGE_BUS_SPEED  POWER_OPS_CHANGE_BUS_SPEED
#define POWER_MGR_OPS_PLL_UPDATE        POWER_OPS_PLL_UPDATE
#define POWER_MGR_OPS_SLEEPING          POWER_OPS_SLEEPING
#define POWER_MGR_MODE_RTC              POWER_MODE_RTC
#define POWER_MGR_MODE_ALWAYSON         POWER_MODE_ALWAYSON
#define POWER_MGR_MODE_FULL             POWER_MODE_FULL
#define POWER_MGR_MODE_RETENTION        POWER_MODE_RETENTION
#define POWER_MGR_MODE_DEEP_RETENTION   POWER_MODE_DEEP_RETENTION
#define power_mgr_softoff               kdrv_power_softoff
#define power_mgr_sw_reset              kdrv_power_sw_reset
#define power_mgr_set_domain            kdrv_power_set_domain
#define power_mgr_ops                   kdrv_power_ops
#define system_init                     kdrv_system_init
#define system_init_ncpu                kdrv_system_init_ncpu
#define kdp_uart_dev_id                 kdrv_uart_dev_id_t
#define kdp_gets(__port, __buf)         kdrv_uart_get_string((kdrv_uart_handle_t)__port, __buf)
#define kdp_uart_GetRxCount             kdrv_uart_get_rx_count
#define kdp_uart_read                   kdrv_uart_read
#define kdp_uart_write                  kdrv_uart_write
#else
#include "kdp520_adc.h" // will be removed in next version
#define kdev_flash_erase_sector kdp_flash_erase_sector
#define kdev_flash_get_status kdp_flash_get_status
#define kdev_flash_programdata kdp_flash_program_data
#define kdev_flash_readdata kdp_flash_read_data
#define kdev_flash_status_t ARM_FLASH_STATUS
#define kdev_status_t kdp_status_t
#define kdrv_power_sw_reset power_mgr_sw_reset
#endif

int kdp_drv_init(void);


#endif
