#ifndef __KDP_FLASH_TABLE_H__
#define __KDP_FLASH_TABLE_H__


#define KDP_FLASH_BOOT_SPL_ADDR             0x00000000 // file=./flash_bin/boot_spl.bin
#define KDP_FLASH_BOOT_SPL_SIZE             0x00002000
#define KDP_FLASH_FW_SCPU_ADDR              0x00002000 // file=./flash_bin/fw_scpu.bin
#define KDP_FLASH_FW_SCPU_SIZE              0x00028000
#define KDP_FLASH_FW_NCPU_ADDR              0x0002A000 // file=./flash_bin/fw_ncpu.bin
#define KDP_FLASH_FW_NCPU_SIZE              0x00010000
#define KDP_FLASH_BOOT_CFG0_ADDR            0x0003A000 // file=./flash_bin/boot_cfg0.bin
#define KDP_FLASH_BOOT_CFG0_SIZE            0x00001000
#define KDP_FLASH_FW_RESERVED1_ADDR         0x0003B000 // file=./flash_bin/fw_reserved.bin
#define KDP_FLASH_FW_RESERVED1_SIZE         0x00006000
#define KDP_FLASH_FW_SCPU1_ADDR             0x00041000 // file=./flash_bin/fw_scpu1.bin
#define KDP_FLASH_FW_SCPU1_SIZE             0x00028000
#define KDP_FLASH_FW_NCPU1_ADDR             0x00069000 // file=./flash_bin/fw_ncpu1.bin
#define KDP_FLASH_FW_NCPU1_SIZE             0x00010000
#define KDP_FLASH_BOOT_CFG1_ADDR            0x00079000 // file=./flash_bin/boot_cfg1.bin
#define KDP_FLASH_BOOT_CFG1_SIZE            0x00001000
#define KDP_FLASH_FID_MAP_ADDR              0x0007A000 // file=./flash_bin/fid_map.bin
#define KDP_FLASH_FID_MAP_SIZE              0x000C8000
#define KDP_FLASH_SETTINGS_ADDR             0x00142000 // file=./flash_bin/setting.bin
#define KDP_FLASH_SETTINGS_SIZE             0x00001000
#define KDP_FLASH_FW_INFO_ADDR              0x00143000 // file=./flash_bin/fw_info.bin
#define KDP_FLASH_FW_INFO_SIZE              0x000001F4
#define KDP_FLASH_ALL_MODELS_ADDR           0x00144000 // file=./flash_bin/all_models.bin
#define KDP_FLASH_ALL_MODELS_SIZE           0x00E8C540
#define KDP_FLASH_INF_ADDR                  0x00FD1000
#define KDP_FLASH_INF_SIZE                  0x00002000

#define KDP_FLASH_LAST_ADDR                 0x00FD3000

#endif
