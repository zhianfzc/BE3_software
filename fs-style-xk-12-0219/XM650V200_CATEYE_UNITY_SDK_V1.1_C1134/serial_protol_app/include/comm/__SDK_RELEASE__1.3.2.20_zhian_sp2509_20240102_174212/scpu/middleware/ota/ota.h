#ifndef __OTA_H__
#define __OTA_H__
#include "board_kl520.h"
#include "kl520_com.h"

#include "kdp_flash_def.h"
#include "usr_flash_img_table.h"

#define OTA_BOOT_CONFIG_LOG         (YES)
#define OTA_CONFIG_LOG_EN           (NO)
#define OTA_LOG_EN                  (YES)
#define OTA_TIMING_DEBUG_EN         (NO)

#define OTA_FULL_CONSOLE_EN        (NO)


#define OTA_CRC_CHECK_EN            (YES)


#if (IMAGE_SIZE == IMAGE_64MB)
#define OTA_USER_BACKUP             (YES)
#define OTA_USER_BACKUP_SEPERATE    (YES)
#define OTA_USER_BOOT_CHECK         (YES)
#else
#define OTA_USER_BACKUP             (NO)
#define OTA_USER_BACKUP_SEPERATE    (NO)
#endif

//----flash OTA related

#define FLASH_HEAD_RX           (0x78875AA5)
#define FLASH_HEAD_RX_1         ((FLASH_HEAD_RX&0xFF000000)>>24 )
#define FLASH_HEAD_RX_2         ((FLASH_HEAD_RX&0x00FF0000)>>16 )
#define FLASH_HEAD_RX_3         ((FLASH_HEAD_RX&0x0000FF00)>>8 )
#define FLASH_HEAD_RX_4         ((FLASH_HEAD_RX&0x000000FF)>>0 )



#define FLASH_HEAD_TX           (~(0x78875AA5))
#define FLASH_TAIL              (0x7887)
#define FLASH_TAIL_1            (FLASH_TAIL&0xFF)
#define FLASH_TAIL_2            ((FLASH_TAIL>>8)&0xFF)


#define FLASH_GET_DATA_HEAD_TX                  (0xC410C410)


#define FLASH_PACKET_HEAD_BYTE_SIZE             (4)
#define FLASH_PACKET_SECTOR_BYTE_SIZE           (2)
#define FLASH_PACKET_DATA_SIZE_BYTE_SIZE        (2)
#define FLASH_PACKET_DATA_BYTE_SIZE             (1)
#define FLASH_PACKET_RESPONSE_BYTE_SIZE         (1)
#define FLASH_PACKET_ACTION_BYTE_SIZE           (1)
#define FLASH_PACKET_CHECKSUM_BYTE_SIZE         (4)

#define FLASH_PACKET_TAIL_BYTE_SIZE             (2)


//error code
#define PACKET_HEAD_CHECK_ERROR                 (0xE0)
#define PACKET_TAIL_CHECK_ERROR                 (0xE1)

#define FLASH_PACKET_OK                         (0x66)


//cmd code
#define FLASH_PROGRAM_ACT                       (0xF0)
#define FLASH_READ_ACT                          (0xF1)
#define FLASH_ERASE_SECTOR_ACT                  (0xF2)
#define FLASH_DATA_TO_DDR_ACT                   (0xF3)
#define FLASH_LARGE_PROGRAM                     (0xFD)
#define FLASH_UI_MODEL_INFO                     (0xFE)      //tool or host told "UI image", fw_info and "All model" bin file size to 520
#define FLASH_MODEL_SIZE                        (0xFA)      //response partial model size to host

#define	FLASH_CMD_ACT                           (0x00)		//for future use

#define	FLASH_READ_NIR_ACT                      (0xE1)
#define	FLASH_READ_RGB_ACT                      (0xE2)
#define	FLASH_READ_ALL_ACT                      (0xE3)

#define FLASH_CMD_ACT_NUM_INIT                  (0x01)
#define FLASH_CMD_ACT_NUM_NIR                   (0x02)
#define FLASH_CMD_ACT_NUM_RGB                   (0x03)
#define FLASH_CMD_XXXXXXXXXX                    (0x04)
#define FLASH_CMD_ACT_NUM_SW_VERSION            (0x05)//??
#define FLASH_CMD_ACT_NUM_PROC_DONE             (0x06)
#define FLASH_CMD_ACT_NUM_SEL_SCPU              (0x07)
#define FLASH_CMD_ACT_NUM_SEL_NCPU              (0x08)

#define FLASH_CMD_ACT_NUM_SEL_MODEL             (0x09)
#define FLASH_CMD_ACT_NUM_SEL_IMAGE_UPDATE      (0x0A)
#define FLASH_CMD_ACT_NUM_SEL_FW_INFO           (0x0B)
#define FLASH_CMD_ACT_NUM_SEL_MODEL_PARTIAL     (0x0C)



//status code
#define FLASH_STATUS_OK                     (0x7855)
#define FLASH_STATUS_FAIL                   (0xEEEE)	//packet fail
#define FLASH_STATUS_COMPARE_FAIL           (0xEEE0)
#define FLASH_STATUS_RECEIVE_FAIL           (0xEEE1)
#define FLASH_STATUS_PROGRAM_FAIL           (0xEEE2)
#define FLASH_STATUS_RECEIVE_OVER_SIZE_FAIL (0xEEE3)
#define FLASH_STATUS_READ_CONFIG_FAIL       (0xEEE4)
#define FLASH_STATUS_CRC_FAIL               (0xEEE5)
#define FLASH_STATUS_MODEL_AREA_FAIL        (0xEEE6)




//---flash controoler related
//----flash machine status
#define FLASH_DRV_FAIL                      ( 0xE2 )
#define FLASH_DRV_PROGRAM_FAIL              ( 0xE3 )
#define FLASH_DRV_OK                        ( 1 )
#define FLASH_DRV_BUSY                      ( 2 )
#define FLASH_DRV_NOTHING                   ( 0x88 )




//Bin files in flash size and index
#define SCPU_SIZE                           ( KDP_FLASH_FW_SCPU_SIZE )
#define NCPU_SIZE                           ( KDP_FLASH_FW_NCPU_SIZE )
#define BOOT_CFG_SIZE                       ( PARTITION_CFG_SIZE )

#define SCPU0_START_SECTOR_INDEX            (KDP_FLASH_FW_SCPU_ADDR>>12 )
#define SCPU0_END_SECTOR_INDEX              ( ((KDP_FLASH_FW_SCPU_ADDR + SCPU_SIZE)>>12) - 1)
#define NCPU0_START_SECTOR_INDEX            (KDP_FLASH_FW_NCPU_ADDR/4096)
#define NCPU0_END_SECTOR_INDEX              ( ((KDP_FLASH_FW_NCPU_ADDR + KDP_FLASH_FW_NCPU_SIZE)>>12) - 1)
#define BOOT_CFG0_START_SECTOR_INDEX        ( KDP_FLASH_BOOT_CFG0_ADDR>>12 )
#define BOOT_CFG0_END_SECTOR_INDEX          ( KDP_FLASH_BOOT_CFG0_ADDR>>12 )

#define SCPU1_START_SECTOR_INDEX            ( KDP_FLASH_FW_SCPU1_ADDR>>12 )
#define SCPU1_END_SECTOR_INDEX              ( ((KDP_FLASH_FW_SCPU1_ADDR + SCPU_SIZE)>>12) - 1)
#define NCPU1_START_SECTOR_INDEX            ( KDP_FLASH_FW_NCPU1_ADDR>>12 )
#define NCPU1_END_SECTOR_INDEX              ( ((KDP_FLASH_FW_NCPU1_ADDR + NCPU_SIZE)>>12) - 1 )
#define BOOT_CFG1_START_SECTOR_INDEX        ( KDP_FLASH_BOOT_CFG1_ADDR >>12 )
#define BOOT_CFG1_END_SECTOR_INDEX          ( KDP_FLASH_BOOT_CFG1_ADDR >>12 )

#define MODEL_START_SECTOR_INDEX            (KDP_FLASH_ALL_MODELS_ADDR>>12)
#define MODEL_END_SECTOR_INDEX              ( ((KDP_FLASH_ALL_MODELS_ADDR+KDP_FLASH_ALL_MODELS_SIZE)>>12) -1 )
#define USR_IMAGE_START_SECTOR_INDEX        (USR_FLASH_SETTINGS_ADDR>>12)
#define USR_IMAGE_END_SECTOR_INDEX          ( (( USR_FLASH_SETTINGS_ADDR+USR_FLASH_SETTINGS_SIZE ) >> 12) -1 )
#define FW_INFO_START_SECTOR_INDEX          (KDP_FLASH_FW_INFO_ADDR>>12)

#if (IMAGE_SIZE == IMAGE_64MB)
//model update add
#define MODEL_START_SECTOR_INDEX_1            ((KDP_FLASH_ALL_MODELS_ADDR+KDP_FLASH_ALL_MODEL_OFFSET_1)>>12)
#define USR_IMAGE_START_SECTOR_INDEX_1        ((USR_FLASH_SETTINGS_ADDR+USR_FLASH_SETTINGS_OFFSET_1)>>12)
#define FW_INFO_START_SECTOR_INDEX_1          ((KDP_FLASH_FW_INFO_ADDR+KDP_FLASH_FW_INFO_OFFSET_1)>>12)
#endif


#define FLAGS_FLASH_START       (1<<1)
//--------------------------------------------

extern void Drv_test_function_main(void);
extern UINT8    *gu8OTA_Rx_buffer;
extern UINT32   gu8OTA_Rx_buffer_index;
extern UINT8    *gu8OTA_Tx_buffer;
extern UINT32   gu8OTA_Tx_buffer_index;

//--------------------------------------------
enum eflash_flow
{
	eflash_init =  0x00,
	eflash_program = 0xF0,
	eflash_read =  0xF1,
	eflash_erase = 0xF2,
	eflash_store_DDR = 0xF3,
	eflash_model_size = FLASH_MODEL_SIZE ,
	eflash_large_program = FLASH_LARGE_PROGRAM,
	eflash_ui_image_info = FLASH_UI_MODEL_INFO,
	eflash_NIR =   FLASH_READ_NIR_ACT,
	eflash_RGB =   FLASH_READ_RGB_ACT,
 	eflash_ALL =   FLASH_READ_ALL_ACT,   
	eflash_idle =  0xFF ,
};

enum eOTA_flow
{

    eOTA_init = 0,          //clear Uart buffer and flags!
    eOTA_packet_analyze,
    eOTA_response_doing,
    eOTA_error_report,
    eOTA_idle,

};

enum eOTA_bin
{
    eOTA_bin_scpu = 0,
    eOTA_bin_ncpu,
    eOTA_bin_all_model,
    eOTA_bin_ui_image,
    eOTA_bin_fw_info,
    eOTA_bin_partial_model,
    eOTA_bin_null,
};



struct st_OTA_format
{
    UINT32  head;                   //packet start
    UINT16  host_number;
    UINT16  cmd_stat;
    UINT16  sector_index;
    UINT8   action_number;
    UINT16  data_size;              //how many bytes need to read or program
    UINT32  check_sum;
    UINT16  tail;                   //packet end
    UINT16	status;                 //response use
    //packet related
    UINT8   busy_flag;
    UINT8   *ptr;
    UINT32  *ptr_index;
    UINT16  data_offset;
    UINT32  buf_max_size;
    //UINT16    data_end_offset;
    UINT32  packet_ddr_ptr;
};

typedef struct st_OTA_flash
{
    enum eOTA_flow flow;
    struct st_OTA_format 	receive_cmd;
    struct st_OTA_format 	send_response;

    //for packet use
    UINT32	*temp_buffer;
    UINT32	temp_buffer_index;
    UINT32	temp_buffer_size;

    //boot info and offset
    UINT32  pass_sector_count;          //passed sector count
    UINT32  sector_offset;              //offset in flash address
    UINT16  sector_acc;
    UINT16  target_sectors;             //total bin file need sectors
    enum eOTA_bin   bin_type;                   //0:SCPU, 1:NCPU,2:model,3:user_UIimage, 4:fw_info, 5:partial mode ... , 0xFF:unknown
    UINT32  target_bytes;               //how many bytes in this bin

    UINT16  sn_cpu_status;              //record OTA update status, 0: update done, oxEEE?: fail
                                        //cpu program sequence, for error check
                                        //bit 0: spcu idle flag, bit 1: npcu idle flag,
                                        //bit 2: scpu ready for program, bit 3: ncpu ready for program
                                        //bit 4: spcu program Done flag, bit 5: npcu program done flag,
                                        //bit 6: scpu forbiden, bit 7: ncpu forbiden
    //save packet to DDR
    UINT32  *ddr_ptr;                   //save bin to DDR for program use
    UINT32  ddr_ptr_index;              //how many bytes in ddr_ptr
    UINT32  CRC;

    //model related parameter
    UINT32  second_last_start_add;     //find the last second model address
    UINT32  aligned_offset;
    UINT32  last_start_add;
    UINT32  model_size;

}OTA_FLASHt;

extern OTA_FLASHt stOTA;
extern osThreadId_t com_bus_tid;

/* Please move the following code to kdp_ota */
extern void Drv_OTA_Thread( void );
extern void Drv_OTA_main( void );
extern void Drv_OTA_init(OTA_FLASHt * stota);
void Drv_Uart2_Tx_arrange( void * data_ptr , UINT16 len);
extern UINT16 drv_flash_main( OTA_FLASHt * stota  );
extern void ota_thread_event_set(void);
extern UINT32 drv_read_all_model_crc(void);
UINT32 drv_read_each_model_crc(uint8_t idx);
extern UINT32 Drv_utility_checksum( UINT8 * buf, UINT16 start, UINT16 end );

#endif
