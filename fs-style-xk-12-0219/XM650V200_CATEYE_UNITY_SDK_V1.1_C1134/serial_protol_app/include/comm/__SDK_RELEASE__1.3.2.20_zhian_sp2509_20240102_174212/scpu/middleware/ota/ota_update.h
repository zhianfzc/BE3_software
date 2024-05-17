#ifndef __OTA_UPDATE_H__
#define __OTA_UPDATE_H__
#include "board_kl520.h"
#include "host_msg.h"

#include "kdp_flash_table.h"
#include "kl520_include.h"
#include "ota.h"

#define SCPU_PARTITION0_START_IN_FLASH    KDP_FLASH_FW_SCPU_ADDR
#define NCPU_PARTITION0_START_IN_FLASH    KDP_FLASH_FW_NCPU_ADDR
#define PARTITION_0_CFG_START_IN_FLASH    KDP_FLASH_BOOT_CFG0_ADDR

#define SCPU_PARTITION1_START_IN_FLASH    KDP_FLASH_FW_SCPU1_ADDR
#define NCPU_PARTITION1_START_IN_FLASH    KDP_FLASH_FW_NCPU1_ADDR
#define PARTITION_1_CFG_START_IN_FLASH    KDP_FLASH_BOOT_CFG1_ADDR

#define SCPU_START_ADDRESS          0x10102000
#define NCPU_START_ADDRESS          0x28000000
#define OTA_START_DDR_ADDR          0x60000000

#define SCPU_IMAGE_SIZE             KDP_FLASH_FW_SCPU_SIZE
#define NCPU_IMAGE_SIZE             KDP_FLASH_FW_NCPU_SIZE
#define MODEL_IMAGE_SIZE            KDP_FLASH_ALL_MODELS_SIZE
#define USER_IMG_IMAGE_SIZE         USR_FLASH_SETTINGS_SIZE
#define FW_INFO_IMAGE_SIZE          KDP_FLASH_FW_INFO_SIZE

#define PARTITION_CFG_SIZE          32

#define OTA_UPDATE_SUCCESS 0
#define OTA_AUTH_FAIL 251
#define OTA_FLASH_FAIL 252
#define OTA_DOWNLOAD_FAIL 253

// scpu and ncpu boot config status
#define  BOOT_STATE_CONFIRMED           0x1
#define  BOOT_STATE_FIRST_BOOT          0x2
#define  BOOT_STATE_POST_FIRST_BOOT     0x4
#define  BOOT_STATE_NOT_CONFIRMED       0x8

//user config boot status
#define  USER_STATE_ACTIVE              0x1
#define  USER_STATE_ON_GOING            0x2
#define  USER_STATE_WAIT_ACTIVE         0x4
#define  USER_STATE_INACTIVE            0x8



#define USER_PARTITION_SCPU             (0x00)
#define USER_PARTITION_NCPU             (0x01)
#define USER_PARTITION_FW_INFO          (0x02)
#define USER_PARTITION_MODEL            (0x03)
#define USER_PARTITION_UI_IMG           (0x04)


#define OTA_JUDGE_SIZE                  (5)

#define OTA_AREA_BIT                    (1<<7)
#define OTA_STATUS_BIT                  (0xF)


#define OTA_STAT_NOTHING                (0x0)       //no OTA, no check, means this area is OK
#define OTA_STAT_DO_OTA                 (0x1)       //??
#define OTA_STAT_BOOT_JUDGE_OK          (0x2)       //CRC and flag OK, use new area
#define OTA_STAT_BOOT_JUDGE_OTA_FAIL    (0x3)       //flag error and use org area
#define OTA_STAT_BOOT_JUDGE_CRC_FAIL    (0x4)       //CRC error and use org area


#define SCPU_CRC_START                  (20)
#define SCPU_CRC_OFFSET                 (0)
#define NCPU_CRC_OFFSET                 (4)
#define FW_INFO_CRC_OFFSET              (8)
#define MODEL_CRC_OFFSET                (12)
#define UI_CRC_OFFSET                   (16)




#if (IMAGE_SIZE == IMAGE_64MB)
//-------------------------------------------
//model related     OTA_USER_BACKUP...........
//-------------------------------------------
//#define KDP_FLASH_USER_CFG0_ADDR            (8191*4096)     //area 0 last sector
//#define KDP_FLASH_USER_CFG1_ADDR            (0x02000000)    //area 1 first sector

#define KDP_FLASH_FW_INFO_ADDR_1            (KDP_FLASH_FW_INFO_ADDR+KDP_FLASH_FW_INFO_OFFSET_1/*0x02000000*/)
#define KDP_FLASH_FW_INFO_SIZE_1            (0x0000017C)
#define KDP_FLASH_ALL_MODELS_ADDR_1         (KDP_FLASH_ALL_MODELS_ADDR+KDP_FLASH_ALL_MODEL_OFFSET_1/*0x02000000*/)
#define KDP_FLASH_ALL_MODELS_SIZE_1         (0x01101F30)
//UI image related
#define USR_FLASH_SETTINGS_ADDR_1           (USR_FLASH_SETTINGS_ADDR+KDP_FLASH_USER_OFFSET_1/*0x02000000*/)

//temp add
//#define KDP_FLASH_FW_INFO_OFFSET_1          (0x02000000)
//#define KDP_FLASH_ALL_MODEL_OFFSET_1        (0x02000000)
#define USR_FLASH_SETTINGS_OFFSET_1         (KDP_FLASH_USER_OFFSET_1)//0x02000000)
#endif

typedef struct {
    u32 partition_id;
    u32 seq;
    u32 flag;
} ota_boot_cfg_item_t;

typedef struct {
    ota_boot_cfg_item_t scpu_cfg;
    ota_boot_cfg_item_t ncpu_cfg;
} ota_boot_cfg_t;

typedef struct {
    ota_boot_cfg_item_t fw_info;
    ota_boot_cfg_item_t model;
    ota_boot_cfg_item_t ui_img;
}ota_user_cfg;


int ota_get_active_ncpu_partition(void);
int ota_update_scpu(void);
int ota_update_ncpu(void);
int ota_update_model(u32 size);
int ota_update_case( u8 update_case, u32 data_addr, u32 size );
int ota_update_switch_active_partition(u32 partition);

extern int ota_handle_first_time_boot(void);
extern void ota_init_partition_boot_cfg(void);
extern void ota_burn_in_config( UINT8  partition );
extern int ota_update_scpu_flag_status( void );
extern int ota_update_ncpu_flag_status( void );
extern int ota_get_active_scpu_partition(void);
extern int ota_get_active_ncpu_partition(void);
extern void ota_update_show_config(void);
extern int ota_get_scpu_flag_status( void );
extern int ota_update_force_switch_active_partition( u32 partition );
extern void ota_user_config_init(void);

#if (OTA_USER_BACKUP == YES )
extern void ota_user_config_init_partial( u8 partition , u8 idx);
extern int ota_user_get_active_area_partial( u8 partition );
extern void ota_user_debug_show(void);
extern void ota_user_read_cfg(void);
extern void ota_user_config_init(void);
extern int ota_user_check_on_going_area( u32 partition );
extern int ota_user_select_inactive_area(u32 partition);
extern int ota_user_select_wait_active_area( u32 partition );
#if ( OTA_USER_BACKUP_SEPERATE == YES )
extern int ota_user_get_active_area( uint8_t idx );
#else
extern int ota_user_get_active_area( void );
#endif
extern int ota_user_area_boot_check( void );
extern void ota_user_dummy_changes_status(void);
extern int ota_get_wait_active_area(u32 partition );

extern void ota_update_boot_judge(void);

#endif

extern u32 ota_crc32( u8 *buf, size_t size);


#define CMD_UPDATE_FW CMD_OTA_UPDATE
#define kmdw_ota_switch_active_partition ota_update_switch_active_partition     //return value are the same, printf diff data, flash read and write different,
#define kmdw_ota_update_scpu ota_update_scpu                                    //dev ota!!
#define kmdw_ota_update_ncpu ota_update_ncpu                                    //dev ota!!
#define kmdw_ota_update_model ota_update_model                                  //dev ota!!
#define kmdw_ota_init(__arg1, __arg2) ota_handle_first_time_boot()      //these two parameter no use in dev, return value the same

#endif
