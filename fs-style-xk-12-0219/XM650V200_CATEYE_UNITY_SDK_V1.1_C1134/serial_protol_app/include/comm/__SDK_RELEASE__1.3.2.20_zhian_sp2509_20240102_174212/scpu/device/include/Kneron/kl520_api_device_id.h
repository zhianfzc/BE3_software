#pragma once
#include "base.h"
#include "version.h"
#include "ipc.h"

#define NIR_SENSOR_ID   0x132
#define RGB_SENSOR_ID   0x2145
#define FLASH_ID        0xc8
#define LCM_ID          0x8552//0x404 dummy
#define TOUCH_ID        0x82
#define DEVICE_NOT_INIT 0xFFFF

typedef struct __device_id
{
    char device_name[12];
    int id;
} device_id;

typedef struct __system_info
{
    u32 unique_id;
    u32 spl_version;
    struct fw_misc_data fw_scpu_version;
    struct fw_misc_data fw_ncpu_version;
#if ( UART_PROTOCOL_VERSION >= 0x0200 )
    u16 com_protocol_version;
#endif
    int model_count;
    struct kdp_model_s **model_infos;
    device_id device_id_0;
    device_id device_id_1;
    device_id device_id_2;
    device_id device_id_3;
    device_id device_id_4;
    //device_id device_id_5;
    device_id *extra_device_id_array;
    u32 extra_device_cnt;
    //device_id device_id_6;
} system_info;


