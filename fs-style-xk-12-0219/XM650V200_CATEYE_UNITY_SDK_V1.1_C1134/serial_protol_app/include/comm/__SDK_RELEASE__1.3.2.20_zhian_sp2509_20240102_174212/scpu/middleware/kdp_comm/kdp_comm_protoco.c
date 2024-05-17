#include "kdp_comm_protoco.h"

#if CFG_COM_PROTOCOL_TYPE == COM_PROTOCOL_TYPE_LWCOM
#if ( CFG_COM_URT_PROT_TYPE == COM_UART_PROT_KDP )
#include "io.h"
#include "pinmux.h"
#include "framework/init.h"
#include "framework/v2k_image.h"
#include "framework/framework_driver.h"
#include "framework/event.h"
#include "kdp520_ssp.h"
#include "kl520_api_ssp.h"
#include "kdp_memory.h"
#include "kdp_ddr_table.h"
#include "kl520_include.h"
#include "kl520_com.h"
#include "kdp_comm_app.h"
#include "kl520_api_fdfr.h"
#include "user_io.h"

#if ( CFG_FMAP_EXTRA_ENABLE == YES ) && ( CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE )
#include "board_ddr_table.h"
#endif

#if (CFG_COM_UART_MSG == COM_UART_MSG_KDP)
#include "kdp_comm_msg_define.h"
#else
#include "user_comm_msg_define.h"
#endif

//#include "netinet/in.h"

//uint16_t g_nMsgDataSize = 0;
uint16_t g_nMsgPkgSize = 0;
uint8_t* msg_dst = NULL;
#define  MSG_MAX_SIZE   1500

char *str_uart_reply(enum uart_reply_result result)
{
    return _str_uart_reply(result);
}

u16 kl_ntohs(u16 val)
{
    u8* p = (u8*) &val;
    return kl_ntohps(p);
}

u32 kl_ntohl(u32 val)
{
    u8* p = (u8*) &val;
    return kl_ntohpl(p);
}

u16 kl_ntohps(u8* p)
{
#if (NET_BYTE_ORDER == KL_BYTE_ORDER)
#else
    return StreamsToBigEndU16(p);
#endif
}

u32 kl_ntohpl(u8* p)
{
#if (NET_BYTE_ORDER == KL_BYTE_ORDER)
#else
    return StreamsToBigEndU32(p);
#endif
}

u16 kl_htons(u16 val)
{
#if (NET_BYTE_ORDER == KL_BYTE_ORDER)
#else
    return ShortType_BigToSmallEnd(val);
#endif
}

u32 kl_htonl(u32 val)
{
#if (NET_BYTE_ORDER == KL_BYTE_ORDER)
#else
    return IntType_BigToSmallEnd(val);
#endif
}

u16 StreamsToBigEndU16(u8* pData)
{
    return ((pData[0]<<8)|(pData[1]<<0));
}

u32 StreamsToBigEndU32(u8* pData)
{
    return ((pData[0]<<24)|(pData[1]<<16)|(pData[2]<<8)|(pData[3]<<0));
}

uint16_t ShortType_BigToSmallEnd(uint16_t value)
{
    uint16_t temp = ((value>>8)&0xff) | ((value&0xff)<<8);
    return temp;
}
uint32_t IntType_BigToSmallEnd(uint32_t value)
{
    uint32_t temp = ((value>>24)&0xff) | (((value>>16)&0xff)<<8) | (((value>>8)&0xff)<<16) | ((value&0xff)<<24);
    return temp;
}

u8 checksum_cal(u8* p_data, u16 IdxS, u16 IdxE)
{
    int i;
    u8 nChkCode = 0;

    for( i = IdxS; i < IdxE; i++ )
    {
        //dbg_msg_console("check_code: 0x%02X",*(p_data + i) );
        nChkCode ^= *(p_data + i);//((MSG_HEAD>>8)^(MSG_HEAD&0xFF)^KID_NOTE^(READY_DATA_SIZE>>8)^(READY_DATA_SIZE&0xFF)^NID_READY^READY_DATA);
    }

    return nChkCode;
}

#if ( UART_PROTOCOL_VERSION >= 0x0200 )
u16 kdp_comm_get_protocol_version(void)
{
    return UART_PROTOCOL_VERSION;
}
#endif

#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK )
#if ( ENCRYPTION_MODE&AES_ENCRYPTION ) || ( ENCRYPTION_MODE&XOR_ENCRYPTION )
uint8_t  debug_key[KEY_SIZE]  = {0x64,0x30,0x35,0x62,0x30,0x32,0x63,0x30,0x64,0x38,0x31,0x64,0x35,0x64,0x64,0x62};

#if ( ENCRYPTION_MODE&AES_ENCRYPTION )
uint16_t DSM_CheckBodySize( uint16_t body_size)
{
    uint16_t check_body_size = body_size;
    if(body_size%16 != 0)
        check_body_size = (1+(body_size>>4))<<4;
    return check_body_size;
}

void DSM_AesEncrypt(uint8_t* input, uint8_t* output, uint16_t size)
{
    //input:data  output:enc data
    uint16_t s_msg_head = MSG_HEAD;
    uint16_t realSize = ShortType_BigToSmallEnd(size);

    memset(output,0,size+MSG_AES_HEAD_TAIL_SIZE);
    //memcpy(output, &s_msg_head, MSG_AES_HEAD_SIZE);
    //memcpy(output+2, &realSize, MSG_AES_DATA_SIZE);

#ifdef DEV_PKT_LOG_DETAIL
        dbg_msg_nocrlf("Plaintext Tx: ");
        for(u16 i=0; i<size; i++) {
            dbg_msg_nocrlf("%02x ", input[i]);
        }
        dbg_msg_nocrlf("\r\n");
#endif

    //if(output[0]!=0xef || output[1]!=0xaa)
        //      dbg_msg_console("DSM_AesEncrypt000: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x",output[0],output[1],output[2]);
    int ret = aesEncrypt(debug_key, KEY_SIZE, input, output+MSG_AES_HEAD_BIG_SIZE, size);//maybe set msg_head to 0.
        //if(output[0]!=0xef || output[1]!=0xaa)
        //  dbg_msg_console("DSM_AesEncrypt111: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x,size:%d",output[0],output[1],output[2],size);
    *(output+size+MSG_AES_HEAD_BIG_SIZE) = checksum_cal(output, MSG_AES_HEAD_BIG_SIZE, size+MSG_AES_HEAD_BIG_SIZE);
    memcpy(output, &s_msg_head, MSG_AES_HEAD_SIZE);
    memcpy(output+2, &realSize, MSG_AES_DATA_SIZE);
        if(output[0]!=COM_BUS_HEAD_RX_1 || output[1]!=COM_BUS_HEAD_RX_2)
                dbg_msg_com("DSM_AesEncrypt222: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x",output[0],output[1],output[2]);
    g_nMsgPkgSize += MSG_AES_HEAD_TAIL_SIZE;
}
#endif

#if ( ENCRYPTION_MODE&XOR_ENCRYPTION )
void DSM_XOREncrypt(uint8_t* input, uint8_t* output, uint16_t size)
{
    //input:data  output:enc data
    uint16_t s_msg_head = MSG_HEAD;
    uint16_t realSize = ShortType_BigToSmallEnd(size);

    memset(output,0,size+MSG_AES_HEAD_TAIL_SIZE);
    //memcpy(output, &s_msg_head, MSG_AES_HEAD_SIZE);
    //memcpy(output+2, &realSize, MSG_AES_DATA_SIZE);

#ifdef DEV_PKT_LOG_DETAIL
        dbg_msg_nocrlf("Plaintext Tx: ");
        for(u16 i=0; i<size; i++) {
            dbg_msg_nocrlf("%02x ", input[i]);
        }
        dbg_msg_nocrlf("\r\n");
#endif

    //if(output[0]!=0xef || output[1]!=0xaa)
        //      dbg_msg_console("DSM_AesEncrypt000: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x",output[0],output[1],output[2]);
    int ret = encBytes( input, size, debug_key, 16, output+MSG_AES_HEAD_BIG_SIZE );

        //if(output[0]!=0xef || output[1]!=0xaa)
        //  dbg_msg_console("DSM_AesEncrypt111: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x,size:%d",output[0],output[1],output[2],size);
    *(output+size+MSG_AES_HEAD_BIG_SIZE) = checksum_cal(output, MSG_AES_HEAD_BIG_SIZE, size+MSG_AES_HEAD_BIG_SIZE);
    memcpy(output, &s_msg_head, MSG_AES_HEAD_SIZE);
    memcpy(output+2, &realSize, MSG_AES_DATA_SIZE);
    if(output[0]!=COM_BUS_HEAD_RX_1 || output[1]!=COM_BUS_HEAD_RX_2)
                dbg_msg_com("DSM_AesEncrypt222: output[0]:0x%02x,output[1]:0x%02x,output[2]:0x%02x",output[0],output[1],output[2]);
        //dbg_msg_com("DSM_AesEncrypt checksum:%02x", *(output+size+MSG_AES_HEAD_BIG_SIZE));
    g_nMsgPkgSize += MSG_AES_HEAD_TAIL_SIZE;

}
#endif

static void PkgEncrypt(u8* p_data, uint16_t data_size)
{
#if ( ENCRYPTION_MODE&XOR_ENCRYPTION )&&( ENCRYPTION_MODE&AES_ENCRYPTION )
    if ( g_nEncryptionMode == XOR_ENCRYPTION )
    {
        g_nMsgPkgSize = data_size + MSG_HEAD_BIG_SIZE;
        DSM_XOREncrypt(p_data+MSG_HEAD_SIZE, msg_dst, g_nMsgPkgSize);
    }
    else
    {
        g_nMsgPkgSize = DSM_CheckBodySize(data_size + MSG_HEAD_BIG_SIZE);
        DSM_AesEncrypt(p_data+MSG_HEAD_SIZE, msg_dst, g_nMsgPkgSize);
    }
#elif ( ENCRYPTION_MODE&AES_ENCRYPTION )
    g_nMsgPkgSize = DSM_CheckBodySize(data_size + MSG_HEAD_BIG_SIZE);
    DSM_AesEncrypt(p_data+MSG_HEAD_SIZE, msg_dst, g_nMsgPkgSize);
#elif ( ENCRYPTION_MODE&XOR_ENCRYPTION )
    g_nMsgPkgSize = data_size + MSG_HEAD_BIG_SIZE;
    DSM_XOREncrypt(p_data+MSG_HEAD_SIZE, msg_dst, g_nMsgPkgSize);
#endif
}

void PkgDecrypt(uint8_t *bytes, int length, const uint8_t *key, int stride, uint8_t *out)
{
#if ( ENCRYPTION_MODE&XOR_ENCRYPTION )&&( ENCRYPTION_MODE&AES_ENCRYPTION )
    if ( g_nEncryptionMode == XOR_ENCRYPTION )
    {
        DecBytes( bytes, length, key, stride, out );
    }
    else
    {
        aesDecrypt(key, stride, bytes, out, length);
    }
#elif ( ENCRYPTION_MODE&AES_ENCRYPTION )
    aesDecrypt(key, stride, bytes, out, length);
#elif ( ENCRYPTION_MODE&XOR_ENCRYPTION )
    DecBytes( bytes, length, key, stride, out );
#endif
}
#endif

static void kdp_com_fill_hdr(msg_base* p, uint8_t cmd, uint16_t size)
{
    if(p == NULL) return;
    p->head = MSG_HEAD;
    p->cmd = cmd;
    p->size = kl_htons(size);
}

static void kdp_com_pack_rsp_msg(s_msg_rsp_base* p, uint8_t cmd, uint16_t size, uint8_t kid, uint8_t result)
{
    if(p == NULL) return;
    kdp_com_fill_hdr(&(p->msg_head), cmd, size);
    
    p->kid = kid;
    p->result = result;
    return;
}

static void kdp_com_send_out_msg(uint8_t* msg_ptr, int msg_size)
{
    if(msg_size < (sizeof(msg_base) + 1)) return; //error size

    if (g_nEncryptionMode == NO_ENCRYPTION)
    {
        //msg.check
        msg_ptr[msg_size - 1] = checksum_cal(msg_ptr, MSG_HEAD_SIZE, msg_size - MSG_CHK_SIZE);
        user_com_response_data(msg_ptr, msg_size);
    }
#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
    else
    {
        uint16_t data_size = msg_size - 1 - sizeof(msg_base);
        PkgEncrypt(msg_ptr, data_size);
        user_com_response_data(msg_dst, g_nMsgPkgSize);
    }
#endif
}


void send_reply_AesNoDataMsg(uint8_t result, uint8_t kid)// MR_SUCCESS; mid:reply type msg
{
    return send_reply_NoDataMsg(result, kid);
}

void send_reply_NoDataMsg(uint8_t result,uint8_t kid)// MR_SUCCESS; mid:reply type msg
{
    s_msg_reply_no_data reply_snapImage;

    uint16_t size = MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_snapImage, KID_REPLY, size, kid, result);
    
    //no data

    kdp_com_send_out_msg((uint8_t*)&reply_snapImage, sizeof(reply_snapImage));
}

void send_system_ready_note_msg(void)
{
    s_msg_note_ready note_ready;

    uint16_t msg_size = MSG_APPEND_1_SIZE;
    kdp_com_fill_hdr((msg_base*)&note_ready, KID_NOTE, msg_size);

#ifdef GET_HOST_CAMERA_STATUS
    if(GET_HOST_CAMERA_STATUS())
        note_ready.nid = NID_CATEYE_RUNNING;
    else
#endif
    {
        note_ready.nid = NID_READY;
    }

    kdp_com_send_out_msg((uint8_t*)&note_ready, sizeof(note_ready));
}

//void send_communication_abnormal_reply_msg(uint8_t result, uint16_t status)
//{
//    dbg_msg_console("%s error code: 0x%04X", __func__, status);

//    s_msg_reply_communication_abnormal packet_abnormal;
//    uint8_t* msg_ptr = (uint8_t*)&packet_abnormal;

//    packet_abnormal.msg_head.head = MSG_HEAD;
//    packet_abnormal.msg_head.cmd = KID_ERROR;
//    packet_abnormal.msg_head.size = ShortType_BigToSmallEnd(MSG_APPEND_3_SIZE);
//    packet_abnormal.result = result;
//    packet_abnormal.status[0] = (status>>8)&0xFF;
//    packet_abnormal.status[1] = (status>>0)&0xFF;;

//    if ( g_nEncryptionMode == NO_ENCRYPTION )
//    {
//        packet_abnormal.check = checksum_cal(msg_ptr, MSG_HEAD_SIZE, sizeof(packet_abnormal)-MSG_CHK_SIZE);
//        user_com_response_data( msg_ptr, sizeof(packet_abnormal));//
//    }
//#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
//    else
//    {
//        PkgEncrypt(msg_ptr);
//        user_com_response_data( msg_dst, g_nMsgPkgSize);
//    }
//#endif
//}

void send_heartbeat_msg(void)
{
    //send hb msg to MCU.
//    s_msg_note_ready hb_msg;
//    uint8_t* msg_ptr = (uint8_t*)&hb_msg;
//    hb_msg.msg_head.head = 0x1234;
//    hb_msg.msg_head.cmd = 0x00;
//    hb_msg.msg_head.size = ShortType_BigToSmallEnd(MSG_APPEND_1_SIZE);
//    hb_msg.nid = 0xab;
//    hb_msg.check = checksum_cal(msg_ptr, MSG_HEAD_SIZE, sizeof(hb_msg)-MSG_CHK_SIZE);
    
    u16 msg = 0x1234;
    user_com_response_data( (uint8_t*)&msg, sizeof(msg));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x00-0x0F System
#ifdef KID_DEVICE_INFO
void send_GetDeviceInfo_reply_msg(uint8_t result,device_info_data device_info)
{
    s_msg_reply_device_info_data msg_device_info;
    uint8_t* msg_ptr = (uint8_t*)&msg_device_info;
    msg_device_info.msg_head.head = MSG_HEAD;
    msg_device_info.msg_head.cmd = KID_REPLY;
    msg_device_info.msg_head.size = ShortType_BigToSmallEnd(sizeof(msg_device_info.device_info)+MSG_APPEND_2_SIZE);//sizeof(logsize)+mid+result
    msg_device_info.kid = KID_DEVICE_INFO;//DEVICE_INFO;
    msg_device_info.result = result;
    msg_device_info.check = msg_device_info.msg_head.cmd;

    memcpy(&msg_device_info.device_info,&device_info,sizeof(device_info));

    if (g_nEncryptionMode == NO_ENCRYPTION)
    {
        msg_device_info.check = checksum_cal(msg_ptr, MSG_HEAD_SIZE, sizeof(msg_device_info) - MSG_CHK_SIZE);
        dbg_msg_com("%s and check:0x%02x", __func__, debug_info.check);

        user_com_response_data(msg_ptr, sizeof(msg_device_info));
    }
#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
    else
    {
        PkgEncrypt(msg_ptr);
        user_com_response_data(msg_dst, g_nMsgPkgSize);
    }
#endif
}
#endif

#ifdef KID_KN_DEVICE_INFO
void send_Get_Kn_DeviceInfo_reply_msg(uint8_t result, kn_device_info_data *device_info)
{
    s_msg_reply_kn_device_info_data msg_device_info;

    uint16_t size = sizeof(msg_device_info.device_info) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_device_info, KID_REPLY, size, KID_KN_DEVICE_INFO, result);

    memcpy(&msg_device_info.device_info, device_info, sizeof(kn_device_info_data));

    kdp_com_send_out_msg((uint8_t*)&msg_device_info, sizeof(msg_device_info));
}
#endif

#ifdef KID_GET_VERSION
void response_get_version_info_msg(uint8_t result, uint8_t Version[32]) //result:MR_SUCCESS
{
    s_msg_reply_version_data version_info;

    uint16_t size = sizeof(version_info.version_infos) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&version_info, KID_REPLY, size, KID_GET_VERSION, result);
   
    memcpy(version_info.version_infos,Version,VERSION_INFO_BUFFER_SIZE);

    kdp_com_send_out_msg((uint8_t*)&version_info, sizeof(version_info));
}
#endif
#ifdef KID_GET_VERSION_ZA
void response_get_version_info_msg_zhian(uint8_t result, uint8_t Version[32]) //result:MR_SUCCESS
{
    s_msg_reply_version_data version_info;

    uint16_t size = sizeof(version_info.version_infos) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&version_info, KID_REPLY, size, KID_GET_VERSION_ZA, result);
    memset(version_info.version_infos, 0, VERSION_INFO_BUFFER_SIZE);//zcy add 
    memcpy(version_info.version_infos,Version,VERSION_INFO_BUFFER_SIZE);

    kdp_com_send_out_msg((uint8_t*)&version_info, sizeof(version_info));
}
#endif
#ifdef KID_GET_VERSION_ZAPRD
void response_get_version_info_msg_zhian_prd(uint8_t result, uint8_t Version[32]) //result:MR_SUCCESS
{
    s_msg_reply_version_data version_info;

    uint16_t size = sizeof(version_info.version_infos) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&version_info, KID_REPLY, size, KID_GET_VERSION_ZAPRD, result);
    memset(version_info.version_infos, 0, VERSION_INFO_BUFFER_SIZE);//zcy add 
    memcpy(version_info.version_infos,Version,VERSION_INFO_BUFFER_SIZE);

    kdp_com_send_out_msg((uint8_t*)&version_info, sizeof(version_info));
}
#endif

#ifdef KID_GET_VERSION_HARDWARE
void response_get_version_info_msg_zhian_hard(uint8_t result, uint8_t Version[32]) //result:MR_SUCCESS
{
    s_msg_reply_version_data version_info;

    uint16_t size = sizeof(version_info.version_infos) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&version_info, KID_REPLY, size, KID_GET_VERSION_HARDWARE, result);
    memset(version_info.version_infos, 0, VERSION_INFO_BUFFER_SIZE);//zcy add 
    memcpy(version_info.version_infos,Version,VERSION_INFO_BUFFER_SIZE);

    kdp_com_send_out_msg((uint8_t*)&version_info, sizeof(version_info));
}
#endif

void send_DebugModeOrDemoMode_reply_msg(uint8_t result, uint8_t kid) // MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
{
    dbg_msg_com("%s", __func__);

    send_reply_AesNoDataMsg(result, kid);
}

#ifdef KID_GET_DEBUG_INFO
void send_GetDebugInfo_reply_msg(uint8_t result, uint8_t debug_file_size[4])
{
    s_msg_reply_get_debug_info_data  debug_info;

    uint16_t size = sizeof(debug_info.debug_file_size) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&debug_info, KID_REPLY, size, KID_GET_DEBUG_INFO, result);

    memcpy(debug_info.debug_file_size,debug_file_size,sizeof(debug_info.debug_file_size));

    kdp_com_send_out_msg((uint8_t*)&debug_info, sizeof(debug_info));
}
#endif

#ifdef KID_UPLOAD_DEBUG_INFO
void send_UploadDebugInfo_reply_msg(uint8_t result, uint8_t upload_debug_info_offset[4], uint8_t upload_debug_info_size[4]) //log_size up to 4Kbytes
{
    s_msg_reply_upload_debug_info_data upload_log_data;

    uint16_t size = sizeof(upload_log_data.upload_debug_info_offset) + sizeof(upload_log_data.upload_debug_info_size) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&upload_log_data, KID_REPLY, size, KID_UPLOAD_DEBUG_INFO, result);

    memcpy(upload_log_data.upload_debug_info_offset,upload_debug_info_offset,sizeof(upload_log_data.upload_debug_info_offset));
    memcpy(upload_log_data.upload_debug_info_size,  upload_debug_info_size,  sizeof(upload_log_data.upload_debug_info_size));

    kdp_com_send_out_msg((uint8_t*)&upload_log_data, sizeof(upload_log_data));
}
#endif

#ifdef KID_GET_LOG_FILE
void send_GetLogFile_reply_msg(uint8_t result, uint8_t log_size[4])
{
    s_msg_reply_get_log_data get_log;

    uint16_t size = sizeof(get_log.log_size)+MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&get_log, KID_REPLY, size, KID_GET_LOG_FILE, result);

    memcpy(get_log.log_size,log_size,sizeof(get_log.log_size));

    kdp_com_send_out_msg((uint8_t*)&get_log, sizeof(get_log));
}
#endif

#ifdef KID_UPLOAD_LOG_FILE
void send_UploadLogFile_reply_msg(uint8_t result, uint8_t* logdata, uint16_t log_size) //log_size up to 4Kbytes
{
    s_msg_reply_upload_log_data upload_log_data;

    uint16_t size = log_size + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&upload_log_data, KID_REPLY, size, KID_UPLOAD_LOG_FILE, result);

    memcpy(upload_log_data.log_data,logdata,log_size);

    //seems has problem here. !!!
    kdp_com_send_out_msg((uint8_t*)&upload_log_data, sizeof(upload_log_data));
}
#endif

//#ifdef KID_RESET
//void send_Reset_reply_msg(uint8_t result)// MR_SUCCESS;
//{
//    dbg_msg_com("%s", __func__);
//    send_reply_AesNoDataMsg(result,KID_RESET);
//}
//#endif

#ifdef KID_SOFT_RESET
void send_soft_reset_reply_msg(uint8_t result)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result,KID_SOFT_RESET);
}
#endif

#ifdef KID_POWERDOWN
void send_power_off_reply_msg()
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(MR_SUCCESS,KID_POWERDOWN);
}
#endif

void send_data_error_reply_msg(uint8_t result)
{
    dbg_msg_com("%s", __func__);
//    if ( g_nEncryptionMode == NO_ENCRYPTION )
//    {
//        send_reply_NoDataMsg(result, KID_REPLY);
//    }
//    else
//    {
    send_reply_AesNoDataMsg(result, KID_REPLY);
//    }
}
//-----0x00-0x0F System

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x10-0x3F Function
#ifdef KID_GET_STATUS
void send_status_reply_msg(uint8_t result, uint8_t status)// MR_SUCCESS;
{
    s_msg_reply_getstatus_data reply_status;

    uint16_t msg_size = MSG_APPEND_3_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_status, KID_REPLY, msg_size, KID_GET_STATUS, result);

    reply_status.status = status;

    kdp_com_send_out_msg((uint8_t*)&reply_status, sizeof(reply_status));
}
#endif

//send result-reply msg to H
#ifdef KID_VERIFY
void send_verify_reply_msg(uint8_t result,msg_verify_data userinfo)
{
    s_msg_reply_verify_data msg_reply;

    uint16_t msg_size = sizeof(msg_reply.reply_data) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_reply, KID_REPLY, msg_size, KID_VERIFY, result);

    memcpy(&msg_reply.reply_data, &userinfo, sizeof(userinfo));

    kdp_com_send_out_msg((uint8_t*)&msg_reply, sizeof(msg_reply));

    dbg_msg_console("[%s][%d] result: %s", __func__, osKernelGetTickCount(), str_uart_reply((enum uart_reply_result)result));
    g_nAutoPowerOffCnt = 0;
}
#endif

//send result-reply msg to H
#if defined KID_ENROLL || defined KID_ENROLL_OVERWRITE
void send_enroll_reply_msg(uint8_t result, uint8_t user_id_heb, uint8_t user_id_leb,uint8_t face_direction, uint8_t cmd_id)
{
    s_msg_reply_enroll_data msg_reply;

    uint16_t msg_size = sizeof(msg_reply.reply_data) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_reply, KID_REPLY, msg_size, cmd_id, result);

    msg_reply.reply_data.user_id_heb = user_id_heb;    //alg result
    msg_reply.reply_data.user_id_leb = user_id_leb;    //alg result
    msg_reply.reply_data.face_direction = face_direction; //alg result

    kdp_com_send_out_msg((uint8_t*)&msg_reply, sizeof(msg_reply));

    g_nAutoPowerOffCnt = 0;
    dbg_msg_console("[%s][%d] result: %s", __func__, osKernelGetTickCount(), str_uart_reply((enum uart_reply_result)result));
}
#endif

//send note msg to H
void send_EnrollOrVerify_note_msg(s_msg_note_data_face face_info,uint8_t nid)
{
    s_msg_note_data_EAR_face note_face_data;

    uint16_t msg_size = sizeof(note_face_data.msg_data) + MSG_APPEND_1_SIZE;
    kdp_com_fill_hdr((msg_base*)&note_face_data, KID_NOTE, msg_size);

    note_face_data.nid = nid;
    memcpy(&note_face_data.msg_data, &face_info, sizeof(face_info));

    kdp_com_send_out_msg((uint8_t*)&note_face_data, sizeof(note_face_data));
}

//void send_EnrollFaceSet_reply_msg(uint8_t result)// MR_SUCCESS;
//{
//    dbg_msg_com("%s", __func__);
//    send_reply_AesNoDataMsg(result, KID_ENROLL_SINGLE);
//}

#ifdef KID_FACE_RESET
//void send_FaceReset_reply_msg(uint8_t result)// MR_SUCCESS;
//{
//    dbg_msg_com("%s", __func__);
//    send_reply_AesNoDataMsg(result, KID_FACE_RESET);
//}
#endif

#ifdef KID_DEL_USER
void response_delete_one_msg(uint8_t result, uint16_t user_id)
{
    s_msg_reply_delete_one_data reply_deleteone;

    uint16_t msg_size = 2 + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_deleteone, KID_REPLY, msg_size, KID_DEL_USER, result);

    reply_deleteone.user_id_heb = (user_id >> 8) & 0xFF;
    reply_deleteone.user_id_leb = user_id & 0xFF;

    kdp_com_send_out_msg((uint8_t*)&reply_deleteone, sizeof(reply_deleteone));
}
#endif

void response_delete_msg(uint8_t result, uint8_t OneOrAll)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result,OneOrAll);
}

//MID_GET_ALL_USERID  MID_GETUSERINFO
#ifdef KID_GET_USER_INFO
void response_get_user_info_msg(uint8_t result, msg_get_user_info_data userinfo)          //result:MR_SUCCESS
{
    s_msg_reply_get_user_info msg_userinfo;

    uint16_t msg_size = sizeof(msg_userinfo.user_info) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_userinfo, KID_REPLY, msg_size, KID_GET_USER_INFO, result);

    memcpy(&msg_userinfo.user_info, &userinfo, sizeof(userinfo));

    kdp_com_send_out_msg((uint8_t*)&msg_userinfo, sizeof(msg_userinfo));
}
#endif

#ifdef KID_GET_ALL_USER_ID
void response_get_Alluser_info_msg(uint8_t result, msg_get_all_user_id_data P_allUserInfo)   //result:MR_SUCCESS
{
    s_msg_reply_get_all_user_info msg_alluserinfo;

    uint16_t msg_size = sizeof(msg_alluserinfo.AllUserInfo) - sizeof(msg_get_all_user_id_data) + 1 + P_allUserInfo.user_counts * 2 + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_alluserinfo, KID_REPLY, msg_size, KID_GET_ALL_USER_ID, result);

    memcpy(&msg_alluserinfo.AllUserInfo, &P_allUserInfo, sizeof(P_allUserInfo));     //core provide 20 id

    kdp_com_send_out_msg((uint8_t*)&msg_alluserinfo, sizeof(msg_alluserinfo) - sizeof(msg_get_all_user_id_data) + 1 + P_allUserInfo.user_counts * 2);
}
#endif

#if ( CFG_PRODUCTION_TEST == NO ) || ( IGNORE_PRODUCTION_TEST == YES )

#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
void send_snapImage_reply_msg(uint8_t result)// MR_SUCCESS;
{
#ifdef KID_SNAP_IMAGE
        send_reply_AesNoDataMsg(result, KID_SNAP_IMAGE);
#endif
#ifdef KID_KN_SNAP_IMAGE
        send_reply_AesNoDataMsg(result, KID_KN_SNAP_IMAGE);
#endif
}
#endif

#if defined KID_GET_SAVED_IMAGE || defined KID_KN_GET_SAVED_IMAGE
void send_savedImage_reply_msg(uint8_t result,uint8_t size[4])// MR_SUCCESS; size:number of saved image
{
    s_msg_reply_get_saved_image_data reply_savedImage;

    uint16_t msg_size = sizeof(reply_savedImage.image_size) + MSG_APPEND_2_SIZE;

#ifdef KID_GET_SAVED_IMAGE
        kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_savedImage, KID_REPLY, msg_size, KID_GET_SAVED_IMAGE, result);
#endif
#ifdef KID_KN_GET_SAVED_IMAGE
        kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_savedImage, KID_REPLY, msg_size, KID_KN_GET_SAVED_IMAGE, result);
#endif

    memcpy(reply_savedImage.image_size,size,sizeof(reply_savedImage.image_size));

    kdp_com_send_out_msg((uint8_t*)&reply_savedImage, sizeof(reply_savedImage));
}
#endif

void send_uploadImage_reply_msg(uint8_t result,uint8_t* upload_image_data,uint16_t upload_image_size)
{
    uint8_t* ptr = stCom_type.tx_buffer;

    kdp_com_fill_hdr((msg_base*)ptr, KID_IMAGE, upload_image_size);

    u16 nDataLen = sizeof(msg_base);
    memcpy(ptr + nDataLen, upload_image_data, upload_image_size);
    nDataLen += upload_image_size;
    
    if ( result == MR_SUCCESS ) {
        kdp_com_send_out_msg((uint8_t*)ptr, nDataLen + 1);
    }
}

#endif

#if CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_EXTRA_ENABLE == YES
#ifdef KID_SW_EXP_FM_MODE
void send_switch_exp_fm_mode_reply_msg(uint8_t result)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result, KID_SW_EXP_FM_MODE);    //result: [1]Enable, [0]Disable
}
#endif

#ifdef KID_SW_EXP_DB_MODE
void send_switch_exp_db_mode_reply_msg(uint8_t result)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result, KID_SW_EXP_DB_MODE);    //result: [1]Enable, [0]Disable
}
#endif

#if ( CFG_FMAP_EX_FIG_ENABLE == YES )
void send_catch_image_mode_reply_msg(uint8_t result)
{
    dbg_msg_console("%s", __func__);
    send_reply_AesNoDataMsg(result, KID_CATCH_IMG_MODE);    //result: [1]Enable, [0]Disable
}
#endif

#endif

void send_exp_mass_data_reply_msg(uint8_t result, u8 eCMD, u32 nAddr, u16 nUploadSize)
{
    uint8_t* ptr = stCom_type.tx_buffer;
    uint16_t msg_size = nUploadSize + MSG_APPEND_2_SIZE;

    kdp_com_pack_rsp_msg((s_msg_rsp_base*)ptr, KID_REPLY, msg_size, eCMD, result);

    //Copy to tx buffer
    //kl520_com_buf_addr_init();
    u16 nDataLen = sizeof(msg_base) + MSG_APPEND_2_SIZE;
    memcpy(ptr + nDataLen, (u8*)nAddr, nUploadSize);
    nDataLen += nUploadSize;

    kdp_com_send_out_msg((uint8_t*)ptr, nDataLen + 1);
}

void send_exp_mass_data_done_note_msg(uint8_t nResult)
{
    s_msg_note_mass_data note_mass_data_done;

    uint16_t size = MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&note_mass_data_done, KID_NOTE, size, NID_MASS_DATA_DONE, nResult);

    kdp_com_send_out_msg((uint8_t*)&note_mass_data_done, sizeof(note_mass_data_done));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x40-0x6F Unit control
void send_Response_result_reply_msg(uint8_t result, uint8_t kid)
{
#if ( CFG_PRODUCTION_TEST == YES ) || ( IGNORE_PRODUCTION_TEST == YES )
#if defined(KID_TURN_ON_CAMERA) && defined(KID_TURN_OFF_PANEL)
    char aStrSw[][4] = {"NULL", "ON", "OFF"};
    char aStrDev[][10] = {"NULL", "CAMERA", "VIS_LED", "IR_LED", "STRUCT_LED", "PANEL"};
    u8 nStrSwIdx = 0;
    u8 nStrDevIdx = 0;

    if ( (kid >= KID_TURN_ON_CAMERA) && (kid <= KID_TURN_OFF_PANEL) )
    {
        nStrSwIdx  = ((kid-KID_TURN_ON_CAMERA)&0x01)+1;
        nStrDevIdx = ((kid-KID_TURN_ON_CAMERA)>>1)+1;
        dbg_msg_console("%s, KID:0x%02X, %s_%s", __func__, kid, aStrSw[nStrSwIdx], aStrDev[nStrDevIdx]);
    }
    else
#endif
#endif
    if ( result == MR_REJECTED )
    {
        dbg_msg_com("%s, KID:0x%02x, %s", __func__, kid, "MR_REJECTED");
    }
    else
    {
        dbg_msg_com("%s, KID:0x%02x", __func__, kid);
    }

    send_reply_AesNoDataMsg(result ,kid);
}

void send_SetMassDataHeader_reply_msg(uint8_t result, uint8_t kid)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result, kid);
}

#ifdef KID_SET_THRESHOLD_LEVEL
void send_AlgThreshold_level_reply_msg(uint8_t result) //MR_SUCCESS
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result, KID_SET_THRESHOLD_LEVEL);
}
#endif

#ifdef KID_CONFIG_BAUDRATE
void send_ConfigBaurate_reply_msg(uint8_t result)// MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result,KID_CONFIG_BAUDRATE);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0x70-0x7F MP
#ifdef KID_SYS_INIT_READY_TIME
void send_SysInitReadyTime_reply_msg(uint8_t result, uint8_t time[4])
{
    s_msg_reply_fisrt_frame_time frame_time;
    uint8_t* msg_ptr = (uint8_t*)&frame_time;
    frame_time.msg_head.head = MSG_HEAD;
    frame_time.msg_head.cmd = KID_REPLY;
    frame_time.msg_head.size = ShortType_BigToSmallEnd(sizeof(frame_time.time)+MSG_APPEND_2_SIZE);//sizeof(logsize)+mid+result
    frame_time.kid = KID_SYS_INIT_READY_TIME;
    frame_time.result = result;
    frame_time.check = frame_time.msg_head.cmd;

    memcpy(frame_time.time,time,sizeof(time));

    frame_time.check = checksum_cal(msg_ptr, MSG_HEAD_SIZE, sizeof(frame_time)-MSG_CHK_SIZE);
    dbg_msg_console("%s and check:0x%02x", __func__, frame_time.check);
    user_com_response_data( msg_ptr, sizeof(frame_time));
}
#endif

#if defined KID_SW_BOOT_PART || defined KID_GET_CUR_PART
void send_SwPart_reply_msg(uint8_t result, uint8_t nCurPart, uint8_t kid)
{
    s_msg_reply_switch_part switch_part;

    uint16_t size = sizeof(switch_part.current_part) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&switch_part, KID_REPLY, size, kid, result);

    switch_part.current_part = nCurPart;

    kdp_com_send_out_msg((uint8_t*)&switch_part, sizeof(switch_part));
}
#endif

#ifdef KID_SW_BOOT_PART
void send_SwitchPart_reply_msg(uint8_t result, uint8_t nCurPart)
{
    return send_SwPart_reply_msg(result, nCurPart, KID_SW_BOOT_PART);
}
#endif

#ifdef KID_GET_CUR_PART
void send_GetCurPart_reply_msg(uint8_t result, uint8_t nCurPart)
{
    return send_SwPart_reply_msg(result, nCurPart, KID_GET_CUR_PART);
}
#endif

#ifdef KID_MP_CALIBRATION
void send_MpCalibration_reply_msg(uint8_t result)
{
    dbg_msg_com("%s", __func__);
    send_reply_AesNoDataMsg(result, KID_MP_CALIBRATION);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF0-0xF7 Key
#ifdef KID_INIT_ENCRYPTION
void send_InitEncryption_reply_msg(uint8_t result, uint8_t device_id[DEVICE_ID_NUM])
{
    s_msg_reply_init_encryption_data encryption;

    uint16_t size = sizeof(encryption.device_id) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&encryption, KID_REPLY, size, KID_INIT_ENCRYPTION, result);

    memcpy(encryption.device_id,device_id,sizeof(encryption.device_id));

#if ( ENCRYPTION_MODE != NO_ENCRYPTION )
    if (msg_dst == NULL)
    {
        msg_dst = (uint8_t *)kdp_ddr_reserve(MSG_MAX_SIZE);
        msg_dst += 128;
    }
#endif

    kdp_com_send_out_msg((uint8_t*)&encryption, sizeof(encryption));
}
#endif

void send_SetReleaseOrDebugEncKey_reply_msg(uint8_t result,uint8_t ReleaseOrDebugEncKey)// MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
{
    dbg_msg_com("%s, %d", __func__, result);
    s_msg_reply_no_data reply_snapImage;

    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_snapImage, KID_REPLY, MSG_APPEND_2_SIZE, ReleaseOrDebugEncKey, result);

    reply_snapImage.check = checksum_cal((uint8_t*)&reply_snapImage, MSG_HEAD_SIZE, sizeof(reply_snapImage) - MSG_CHK_SIZE);
    user_com_response_data((uint8_t*)&reply_snapImage, sizeof(reply_snapImage));
}


#ifdef KID_SET_INTER_ACTIVATE
void send_SetInteractivate_reply_msg(uint8_t result,uint8_t kid)// MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
{
    dbg_msg_com("%s", __func__);
    send_reply_NoDataMsg(result,kid);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//-----0xF8-0xFF OTA
void send_StartOrStopOta_reply_msg(uint8_t result,uint8_t StartOrStopOTA)// MR_SUCCESS; MID_START_OTA:MID_STOP_OTA
{
    dbg_msg_com("%s", __func__);
    send_reply_NoDataMsg(result,StartOrStopOTA);
}

#ifdef KID_GET_OTA_STATUS
void send_OtaStatus_reply_msg(uint8_t result,uint8_t ota_status,uint8_t next_pid_e[2])// MR_SUCCESS;
{
    s_msg_reply_get_ota_status_data reply_OtaStatus;

    uint16_t size = sizeof(reply_OtaStatus.next_pid_e) + MSG_APPEND_3_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&reply_OtaStatus, KID_REPLY, size, KID_GET_OTA_STATUS, result);

    reply_OtaStatus.ota_status = ota_status;
    memcpy(reply_OtaStatus.next_pid_e,next_pid_e,sizeof(reply_OtaStatus.next_pid_e));

    kdp_com_send_out_msg((uint8_t*)&reply_OtaStatus, sizeof(reply_OtaStatus));
}
#endif

void send_OtaHeaderOrPacket_reply_msg(uint8_t result,uint8_t HeaderOrPacket)// MR_SUCCESS; MID_OTA_HEADER:MID_OTA_PACKET
{
    dbg_msg_com("%s", __func__);
    send_reply_NoDataMsg(result,HeaderOrPacket);
}

void send_OtaDone_Note_msg(uint8_t OtaResult)//0:OTA sucess,1:OTA fail
{
    s_msg_note_mass_data note_otaDone;

    uint16_t size = MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&note_otaDone, KID_NOTE, size, NID_OTA_DONE, OtaResult);

    kdp_com_send_out_msg((uint8_t*)&note_otaDone, sizeof(note_otaDone));
}


#ifdef KID_DB_EXPORT_REQUEST
void send_db_export_reply_msg(uint8_t result, u16 user_id, u32 total_size, u8 *md5)
{
    s_msg_reply_db_export_request msg_reply;

    uint16_t size = sizeof(msg_reply.data) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_reply, KID_REPLY, size, KID_DB_EXPORT_REQUEST, result);

    msg_reply.data.user_id_heb = (user_id >> 8) & 0xff;
    msg_reply.data.user_id_leb = user_id & 0xff;
    msg_reply.data.total_size[0] = (total_size >> 24) & 0xff;
    msg_reply.data.total_size[1] = (total_size >> 16) & 0xff;
    msg_reply.data.total_size[2] = (total_size >> 8) & 0xff;
    msg_reply.data.total_size[3] = (total_size >> 0) & 0xff;
    memcpy(msg_reply.data.md5, md5, sizeof(msg_reply.data.md5));

    kdp_com_send_out_msg((uint8_t*)&msg_reply, sizeof(msg_reply));
}
#endif

#ifdef KID_UPLOAD_DATA
void send_upload_data_reply_msg(u8 result, u32 addr, u32 size)
{
    uint8_t* ptr = stCom_type.tx_buffer;

    uint16_t msg_size = size + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)ptr, KID_REPLY, msg_size, KID_UPLOAD_DATA, result);

    //Copy to tx buffer
    u16 nDataLen = sizeof(msg_base) + MSG_APPEND_2_SIZE;
    if ((addr > 0) && (size > 0))
    {
        memcpy(ptr + nDataLen, (u8*)addr, size);
        nDataLen += size;
    }

    kdp_com_send_out_msg((uint8_t*)ptr, nDataLen + 1);
}
#endif

#ifdef KID_DB_IMPORT_REQUEST
void send_db_import_request_reply_msg(uint8_t result, uint16_t user_id)
{
    s_msg_reply_db_import_request msg_reply;

    uint16_t size = sizeof(msg_reply.reply_data) + MSG_APPEND_2_SIZE;
    kdp_com_pack_rsp_msg((s_msg_rsp_base*)&msg_reply, KID_REPLY, size, KID_DB_IMPORT_REQUEST, result);

    msg_reply.reply_data.user_id_heb = (user_id >> 8) & 0xFF;
    msg_reply.reply_data.user_id_leb = user_id & 0xFF;

    kdp_com_send_out_msg((uint8_t*)&msg_reply, sizeof(msg_reply));

    dbg_msg_console("[%s][%d] result: %s", __func__, osKernelGetTickCount(), str_uart_reply((enum uart_reply_result)result));
}

#endif

#else
void send_reply_AesNoDataMsg(uint8_t result, uint8_t kid) {}
void send_reply_NoDataMsg(uint8_t result,uint8_t kid) {}
void send_system_ready_note_msg(void) {}
void send_power_off_reply_msg() {}
void send_verify_reply_msg(uint8_t result,msg_verify_data userinfo) {}
void send_enroll_reply_msg(uint8_t result, uint8_t user_id_heb, uint8_t user_id_leb,uint8_t face_direction, uint8_t cmd_id) {}
void send_EnrollOrVerify_note_msg(s_msg_note_data_face face_info,uint8_t nid) {}
void response_delete_one_msg(uint8_t result, uint16_t user_id) {}
void response_delete_msg(uint8_t result, uint8_t OneOrAll) {}
#endif

#endif
#endif
