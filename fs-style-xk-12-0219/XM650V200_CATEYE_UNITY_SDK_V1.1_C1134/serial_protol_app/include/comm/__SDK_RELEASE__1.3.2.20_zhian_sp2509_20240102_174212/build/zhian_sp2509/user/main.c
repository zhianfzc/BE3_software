/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    main.c
 *      Purpose: RTX for Kneron
 *
 *---------------------------------------------------------------------------*/
#include "board_kl520.h"
#include "kdp520_dma.h"
#include "kneron_mozart.h"                // Device header
#include "kneron_mozart_ext.h"
#include "Driver_Common.h"
#include "io.h"
#include "ddr.h"
#include "kdp_uart.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "scu_reg.h"
#include "scu_extreg.h"
#include "bootloader.h"
#include "power.h"
#include "system.h"
#include "clock.h"
#include "pinmux.h"
#include "power_manager.h"
#include "kl520_api_fdfr.h"
#include "framework/framework.h"
#include "drivers.h"
#include "kdp_model.h"
#include "user_io.h"
#include "user_ui.h"
#include "mpu.h"
#include "flash.h"
#include "kdp_ddr_table.h"
#include "kdp_memory.h"
#include "kdp_camera.h"
#include "ota_update.h"
#include "ota.h"
#include "kl520_api_ssp.h"
#include "kl520_api_device_id.h"
#include "usr_ddr_img_table.h"

#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
#include "kl520_api_snapshot.h"
#endif
#if CFG_FMAP_EXTRA_ENABLE == YES
#include "kl520_api_extra_fmap.h"
#endif
#if CFG_CONSOLE_MODE == YES
#include "sample_app_console.h"
#endif

//========================================LWCOM
#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
//--------------------UART
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#include "kl520_com.h"

#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_DEF )
#include "sample_user_com_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "kdp_comm_and_gui_fdr.h"
#elif ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP_USR )
#include "user_comm_and_gui_fdr.h"
#elif ( CFG_COM_USB_PROT_TYPE == COM_UART_PROT_DEF_USR )

#endif
#else
void user_com_init(void) {}
#endif

//--------------------USB
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#if ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_DEF )
#include "host_com.h"
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP )
#include "kdp_host_com.h"
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP_USR )
#include "user_host_com.h"
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_DEF_USR )

#endif
#endif

//--------------------OTG
#if ( CFG_COM_BUS_TYPE&COM_BUS_OTG_MASK )
extern void otg_init(void);
#endif

//========================================KCOMM
#elif CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#include "kcomm.h"
#endif
#else
#include "kl520_com.h"
#endif

#if ( CFG_GUI_ENABLE == YES )
#include "sample_gui_main.h"
#endif



#define MSG_DBG_LOG
#define MSG_MEM_BYTE_WRITE     0

extern void kdp_uart_app_uart0_log(void);
extern void GPIO010_IRQHandler(void);

/**
 * @info(), Get kernel information
 */

int cpu_type = 0;


void msgZhian()
{
    struct fw_misc_data scpu_version;
    kl520_api_get_scpu_version(&scpu_version);

    dbg_msg_console_zhian("zhian tec BP3-X-25 (V%d.%d.%d.%d)-%d", scpu_version.version[0],
                                                       scpu_version.version[1],
                                                       scpu_version.version[2],
                                                       scpu_version.version[3],
                                                       scpu_version.date); 

}
void info(u32 spl_version) {
    char infobuf[16];
    osVersion_t osv;
    osStatus_t status;
    u32 rom_table;

    rom_table = inw(CPU_ID_STS_ADDR);
    if (CPU_ID_SCPU == rom_table) {
        cpu_type = 0;
    }
    else if (CPU_ID_NCPU == rom_table) {
        cpu_type = 1;
    }

    status = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
    if(status == osOK)
    {
        //dbg_msg_console("+-------------------Keil RTX5----------------------+");
        //dbg_msg_console("Kernel Information: %s", infobuf);
        //dbg_msg_console("Kernel Version    : %d", osv.kernel);
        //dbg_msg_console("Kernel API Version: %d", osv.api);
        u32 spl_version = kdp_sys_get_spl_version();
        struct fw_misc_data scpu_version, ncpu_version;
        kl520_api_get_scpu_version(&scpu_version);
        kl520_api_get_ncpu_version(&ncpu_version);
#ifdef CFG_KL520_VERSION
        dbg_msg_console("BP3-X-25 version: %s", (CFG_KL520_VERSION==KL520B)?"B":"A");//mod KL520 to ZF-BP1-C
#endif
        dbg_msg_console("SPL fw version: %x", spl_version);
        dbg_msg_console("SCPU fw version : %d (%d.%d.%d.%d)",   scpu_version.date,
                                                                scpu_version.version[0],
                                                                scpu_version.version[1],
                                                                scpu_version.version[2],
                                                                scpu_version.version[3]);
        dbg_msg_console("NCPU fw version : %d (%d.%d.%d.%d)",   ncpu_version.date,
                                                                ncpu_version.version[0],
                                                                ncpu_version.version[1],
                                                                ncpu_version.version[2],
                                                                ncpu_version.version[3]);
        dbg_msg_console("FW compile time:%s %s", __DATE__, __TIME__);
    }
}

static void comm_init(void)
{
#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM

//--------------------UART/SPI
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_SPI_MASK )
    kl520_com_bus_init();
#endif
//--------------------USB
#if ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
#if ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_DEF )
    host_com_init();
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP ) || ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_KDP_USR )
    user_host_com_init();
#elif ( CFG_COM_USB_PROT_TYPE == COM_USB_PROT_DEF_USR )

#endif
//--------------------OTG
#if ( CFG_COM_BUS_TYPE&COM_BUS_OTG_MASK )
    otg_init();
#endif
#endif



#elif CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_KCOMM

#else //CFG_COM_BUS_TYPE
    kl520_com_bus_init();
#endif
}

static void user_init(void)
{
    user_io_init();
#if ( CFG_MODELS_LOAD_BY_ORDER == NO )    
    user_ui_init();
#endif    
    user_com_init();
}

static void nvic_init(void)
{
    //register IRQ
    NVIC_SetVector((IRQn_Type)GPIO_FTGPIO010_IRQ, (uint32_t)GPIO010_IRQHandler);

    //enable IRQ
    NVIC_EnableIRQ(GPIO_FTGPIO010_IRQ);

#if I2C_INTERRUPT_ENABLE
#if CFG_I2C_0_ENABLE == YES
    NVIC_EnableIRQ((IRQn_Type)IIC_FTIIC010_0_IRQ);
#endif
#if CFG_I2C_1_ENABLE == YES
    NVIC_EnableIRQ((IRQn_Type)IIC_FTIIC010_1_IRQ);
#endif
#if CFG_I2C_2_ENABLE == YES
    NVIC_EnableIRQ((IRQn_Type)IIC_FTIIC010_2_IRQ);
#endif
#if CFG_I2C_3_ENABLE == YES
    NVIC_EnableIRQ((IRQn_Type)IIC_FTIIC010_3_IRQ);
#endif
#endif
}

/**
 * @brief main, main dispatch function
 */

int main(void)
{
    //u32 spl_version;
    kl520_measure_init();
    kl520_measure_stamp(E_MEASURE_MAIN_BOOT);
    //spl_version = kdp_sys_get_spl_version();
    pinmux_init();
    system_init();
    system_init_ncpu();

    kdp_uart_init();
#if CFG_UART0_ENABLE == YES
    kdp_uart_app_uart0_log();   // for log
#endif
    //kdp_ipc_init(TRUE);
    kl520_api_log_set_user_level(CPU_ID_SCPU, LOG_USER);  //scpu
    kl520_api_log_set_user_level(CPU_ID_NCPU, LOG_USER);  //ncpu
    ddr_init(DDR_INIT_ALL);
    kdp_ddr_init(KDP_DDR_HEAP_HEAD_FOR_MALLOC+1, KDP_DDR_HEAP_HEAD_FOR_MALLOC_END); // will remove in next version

    kdp_ipc_init(TRUE);
    #if(OTA_USER_BOOT_CHECK==YES)
    ota_update_boot_judge();
    #endif
    ota_handle_first_time_boot();
    load_ncpu_fw(1);

    SystemCoreClockUpdate();            // System Initialization
    osKernelInitialize();               // Initialize CMSIS-RTOS
    power_manager_init();
#if (CFG_PRODUCTION_TEST == 1)
    if (ota_get_scpu_flag_status() == 1)
        kl520_api_ota_switch_SCPU();
#endif
    kdp_drv_init();
    kdp_camera_init();
    #if( OTA_USER_BACKUP == YES )
    ota_user_area_boot_check();
    #endif
    kl520_api_tasks_init();
    comm_init();
    user_init();

     msgZhian();

#if CFG_CONSOLE_MODE == YES
    sample_doorlock_entry();
#endif

#if (CFG_GUI_ENABLE == YES)
    sample_gui_init();
#endif

    nvic_init();
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_init();
#endif

#if CFG_FMAP_EXTRA_ENABLE == YES
    kl520_api_extra_fmap_init();
#endif
    info(0);

    kl520_measure_stamp(E_MEASURE_OS_START);
    //application is triggered in host_com.c
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    while(1) {
        osDelay(1000);
    }
}


