#ifndef __KDP_MEMXFER_H__
#define __KDP_MEMXFER_H__


#include "types.h"

#define MEMXFER_OPS_NONE 	    0x00
#define MEMXFER_OPS_CPU 	    0x01
#define MEMXFER_OPS_DMA 	    0x02



struct s_kdp_memxfer {
    int (*init)(u8 flash_mode, u8 mem_mode);
    int (*flash_to_ddr)(u32 dst, u32 src, size_t bytes);
    int (*ddr_to_flash)(u32 dst, u32 src, size_t bytes);
    int (*flash_to_ddr_dma)(u32 dst, u32 src, size_t bytes);
    int (*ddr_to_flash_dma)(u32 dst, u32 src, size_t bytes);    
    int (*ddr_to_ddr)(u32 dst, u32 src, size_t bytes);
    int (*flash_sector_erase64k)(u32 addr);
    int (*flash_sector_multi_erase)( u32 start_add, u32 end_add );
    int (*flash_to_niram)(void);
} ;

int kdp_memxfer_init(u8 flash_mode, u8 mem_mode);
int kdp_memxfer_flash_to_ddr(u32 dst, u32 src, size_t bytes);
int kdp_memxfer_ddr_to_flash(u32 dst, u32 src, size_t bytes);
int kdp_memxfer_ddr_to_flash_sector(u32 dst, u32 src, size_t bytes);
int kdp_memxfer_ddr_to_flash_wo_erase(u32 dst, u32 src, size_t bytes);
#if 0
int kdp_memxfer_flash_to_ddr_dma(u32 dst, u32 src, size_t bytes);
int kdp_memxfer_ddr_to_flash_dma(u32 dst, u32 src, size_t bytes);
int kdp_memxfer_ddr_to_ddr(u32 dst, u32 src, size_t bytes);
#endif
int kdp_memxfer_flash_sector_erase4k(u32 addr);
int kdp_memxfer_flash_sector_erase64k(u32 addr);
int kdp_memxfer_flash_sector_multi_erase( u32 start_add, u32 end_add );
int kdp_memxfer_flash_to_niram(void);


#endif
