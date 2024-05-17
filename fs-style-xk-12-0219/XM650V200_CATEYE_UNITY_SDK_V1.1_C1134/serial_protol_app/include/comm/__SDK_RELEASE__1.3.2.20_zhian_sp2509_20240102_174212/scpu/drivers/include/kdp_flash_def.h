#ifndef __KDP_FLASH_H__
#define __KDP_FLASH_H__


#include "board_kl520.h"

#define FLASH_CODE_OPT              (YES)

typedef enum
{
    KDP_STATUS_ERROR = 0,            /**< flash driver status error */
    KDP_STATUS_OK,           /**< flash driver status OK */
} kdp_status_t;

typedef struct _flash_paramter
{
    UINT32  signature;							    //0x00
    UINT8   PTP;								        //0x0C
    UINT8   ID;									        //0x10
    UINT8   erase_4K_support;				    //0x00 => 0x30[1:0];
    UINT32  flash_size_KByte;				    //0x04~0x07 => 0x34~0x37
    UINT16  page_size_Bytes;				    //0x28 => 0x58[7:4]=0x8
    UINT16  sector_size_Bytes;
    UINT32  block_size_Bytes;				    //how many sectors in one block
    UINT16  total_sector_numbers;
}FLASH_PARMATER_T;

/**
\brief Flash Sector information
*/
typedef struct _ARM_FLASH_SECTOR {
  uint32_t start;                       ///< Sector Start address
  uint32_t end;                         ///< Sector End address (start+size-1)
} const ARM_FLASH_SECTOR;

/**
\brief Flash information
*/
typedef struct _ARM_FLASH_INFO {
  ARM_FLASH_SECTOR *sector_info;        ///< Sector layout information (NULL=Uniform sectors)
  uint32_t          sector_count;       ///< Number of sectors
  uint32_t          sector_size;        ///< Uniform sector size in bytes (0=sector_info used) 
  uint32_t          page_size;          ///< Optimal programming page size in bytes
  uint32_t          program_unit;       ///< Smallest programmable unit in bytes
  uint8_t           erased_value;       ///< Contents of erased memory (usually 0xFF)
  uint32_t          flash_size;
} ARM_FLASH_INFO;    
//} const ARM_FLASH_INFO;

/**
\brief Flash Status
*/
typedef struct _ARM_FLASH_STATUS {
  uint32_t busy  : 1;                   ///< Flash busy flag
  uint32_t error : 1;                   ///< Read/Program/Erase error flag (cleared on start of next operation)
} ARM_FLASH_STATUS;



#endif
