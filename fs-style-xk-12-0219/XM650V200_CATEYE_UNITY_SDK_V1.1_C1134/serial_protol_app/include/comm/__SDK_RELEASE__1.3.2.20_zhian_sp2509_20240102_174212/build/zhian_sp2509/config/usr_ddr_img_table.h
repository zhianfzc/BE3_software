#ifndef __USR_DDR_IMG_TABLE_H__
#define __USR_DDR_IMG_TABLE_H__


#define USR_DDR_SETTINGS_ADDR                               (KDP_DDR_LAST_ADDR + 0x00000000) // 0x61F84A00, file=flash_bin\usr_settings.bin
#define USR_DDR_SETTINGS_SIZE                               0x00001000
#define USR_DDR_LAST_ADDR                                   0x61F85A00

/*

//Copy the content of this annotation to kl520_api_ddr_img_user()
kdp_memxfer_flash_to_ddr(USR_DDR_SETTINGS_ADDR, USR_FLASH_SETTINGS_ADDR, USR_FLASH_SETTINGS_SIZE);
*/ 

#define KDP_DDR_HEAP_HEAD_FOR_MALLOC_END                    (0x61F85A00 - 1)
#define KDP_DDR_HEAP_HEAD_FOR_MALLOC                        0x63FFFFFF

#endif
