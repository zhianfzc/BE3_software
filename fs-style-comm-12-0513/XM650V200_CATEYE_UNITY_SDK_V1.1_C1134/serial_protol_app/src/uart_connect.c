/*******************************************************************************
Copyright @XmSilicon Tech Co., Ltd. 2022-2022. All rights reserved.
文件名: uart_connect.c
作者@编号： XM
功能描述: 人脸模组通讯入口
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <board.h>

#include "st_lock_msg.h"
#include "uart.h"
#include "config.h"

#include "aes.h"

#define UART_PRT1  0 //zcy print uart rec
#define UART_PRT  0 //zcy add for print 1,print 0 no print
#define READY_LOOP 0 // loop send ready until get command ok
extern unsigned long long g_start_pts;
extern rt_device_t serial;
extern int st_get_jpg_save(unsigned char image_number, ST_SIZE_S *pst_pic_size, unsigned int *psend_image_buf_size, unsigned char **p_send_image_buf);

#ifdef DEVICE_VERT
/* 图像分辨率 */
ST_SIZE_S gstPicSize = {
	.u32Width = 576,
	.u32Height = 720,
};
#else
/* 图像分辨率 */
ST_SIZE_S gstPicSize = {
	.u32Width = 1280,
	.u32Height = 720,
};
#endif

unsigned long long g_ready_pts = 0;
static char g_cap_sign = 0;

char *cap_sign_addr_get()
{
	return &g_cap_sign;
}

static uint8_t 			g_verify_timeout = 0;
static uint8_t 			g_enroll_timeout = 0;

static uint8_t 			g_itg_enroll_type = 0;

static uint8_t 			*gp_send_image_buf = NULL;
static uint8_t 			g_send_image_number = 0;
static unsigned int 	g_send_image_buf_size = 0;
static unsigned int 	g_upload_image_offset = 0;
static unsigned int 	g_upload_image_size = 0;

static uint8_t 			g_ota_mode = 0;
static unsigned int 	g_ota_sum_size = 0;
static unsigned int 	g_ota_sum_packet = 0;
static uint16_t 		g_ota_signle_psize = 0;
static uint8_t 			*gp_ota_data_puf = NULL;
static uint8_t 			g_ota_type = 0;

static uint8_t			g_five_input_face_direction = 0;

static uint8_t 			g_device_static = 0;
static uint8_t 			g_demo_mode = 0;

static uint8_t			g_enc_mode = 0;
static uint8_t			g_enc_mode_debug = 0;
static uint8_t			g_key_rule[KEY_SIZE] = {0};	/* 密钥抽取规则 */
static uint8_t			g_enc_key[KEY_SIZE] = {0};	/* 密钥 */


unsigned char verify_timeout_get()
{
	return g_verify_timeout;
}

unsigned char *enroll_timeout_get()
{
	return &g_enroll_timeout;
}

unsigned char *ota_mode_get()
{
	return &g_ota_mode;
}

unsigned char demo_mode_get()
{
	return g_demo_mode;
}

unsigned char five_input_face_direction_get()
{
	return g_five_input_face_direction;
}

/*********************************
Function:send image data
**********************************/
static int st_image_date_send(uint8_t *data)
{
	if (gp_send_image_buf == NULL)
	{
		MY_ERR_PRT("st_image_date_send error:gp_send_image_buf is NULL ", MY_ERRROR);
	}
	if (g_upload_image_offset > g_send_image_buf_size)
	{
		MY_ERR_PRT("upload_image_offset over send_image_buf_size", g_upload_image_offset);
	}

	memcpy(data, gp_send_image_buf+g_upload_image_offset, g_upload_image_size);

	if (g_upload_image_offset+g_upload_image_size == g_send_image_buf_size)
	{
		g_upload_image_offset = 0;
		g_send_image_buf_size = 0;
		free(gp_send_image_buf);
		gp_send_image_buf = NULL;
		
		if (++g_send_image_number == 2)
		{
			g_cap_sign = SIGN_DISABLE; /* 抓图结束 */	
			g_device_static = MS_STANDBY;
			g_send_image_number = 0;
		}
	}

	return 0;
}

/* 数据解析获取 timeout:0 为阻塞状态*/
static int host_msg_get(char data[], int timeout, unsigned int recv_size)
{
    char ch;
    unsigned int i = 0;
	int ret = 0;
    while (1) {
        ret = uart_sample_get_char(&ch, timeout);
        if (ret != 0)
        	return -1;

        data[i++] = ch;
        #if UART_PRT1 //zcy add prt
        	printf("data[%d] 0x%02x", i-1, data[i-1]);
        #endif

		if (i == recv_size)
			break;
    }

    return 0;
}

/*********************************
Function:ota data splice
**********************************/
static int ota_data_splice(uint16_t packet_pid, uint16_t psize, uint8_t *pst_packet_data)
{
	if (!packet_pid)
	{
		//gp_ota_data_puf = (uint8_t*)malloc(g_ota_sum_size);	
		CHECK_RET(ota_mmz_alloc(g_ota_sum_size, &gp_ota_data_puf), "ota_mmz_alloc");
		if (gp_ota_data_puf == NULL)
		{
			MY_ERR_PRT("gp_ota_data_puf malloc error", MY_ERRROR);
			return MY_ERRROR;
		}
	}

	memcpy(gp_ota_data_puf+packet_pid*g_ota_signle_psize, pst_packet_data, psize);

	return 0;
}

static int st_mod_to_host_reply(uint8_t nid, uint8_t reply_result)
{
	int ret;
	unsigned int reply_len = 0;
	/* 获取reply_len */
	switch (nid)
	{
		case MID_RESET:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_GETSTATUS:
			reply_len = sizeof(s_msg_reply_getstatus_data) + sizeof(s_msg_reply_data);
			break;

		case MID_VERIFY:
			if (reply_result == MR_SUCCESS)
				reply_len = sizeof(s_msg_reply_verify_data) + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_ENROLL:
			if (reply_result == MR_SUCCESS)
				reply_len = sizeof(s_msg_reply_enroll_data) + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_ENROLL_SINGLE:
			if (reply_result == MR_SUCCESS)
				reply_len = sizeof(s_msg_reply_enroll_data) + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_ENROLL_ITG:
			if (reply_result == MR_SUCCESS)
				reply_len = sizeof(s_msg_reply_enroll_data) + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_SNAPIMAGE:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_GETSAVEDIMAGE:
			if (reply_result == MR_SUCCESS)
				reply_len = sizeof(s_msg_reply_get_saved_image_data) + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_UPLOADIMAGE:
			if (reply_result == MR_SUCCESS)
				reply_len = g_upload_image_size + sizeof(s_msg_reply_data);
			else
				reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_DELUSER:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_DELALL:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_FACERESET:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_GET_ALL_USERID:
			reply_len = sizeof(s_msg_reply_all_userid_data) + sizeof(s_msg_reply_data);
			break;

		case MID_GET_VERSION:
		case MID_GET_VERSION_ZA:
		case MID_GET_VERSION_ZAPRD:
			reply_len = sizeof(s_msg_reply_version_data) + sizeof(s_msg_reply_data);
			break;

		case MID_START_OTA:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_INIT_ENCRYPTION:
			reply_len = sizeof(s_msg_reply_init_encryption_data) + sizeof(s_msg_reply_data);
			break;

		case MID_CONFIG_BAUDRATE:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_SET_RELEASE_ENC_KEY:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_SET_DEBUG_ENC_KEY:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_OTA_HEADER:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_GET_OTA_STATUS:
			reply_len = sizeof(s_msg_reply_getotastatus_data) + sizeof(s_msg_reply_data);
			break;

		case MID_OTA_PACKET:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_SET_UVC_NET_FLAG:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_SET_THRESHOLD_LEVEL:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_POWERDOWN:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_DEMOMODE:
			reply_len = sizeof(s_msg_reply_data);
			break;

		case MID_GET_INIT_TIME:
			reply_len = sizeof(s_msg_reply_init_time) + sizeof(s_msg_reply_data);
			break;

		default:
			MY_ERR_PRT("public_mid_reply not support nid", nid);
			break;
	}

	unsigned int msg_len = sizeof(s_msg) + reply_len ;
	//zcy add 
	unsigned int msg_buf_len = msg_len + 64;//zcy andd for test
	s_msg *pst_msg = (s_msg*)malloc(msg_buf_len);
	
	memset(pst_msg, 0, msg_buf_len);
	
	s_msg_reply_data *pst_msg_reply = (s_msg_reply_data*)pst_msg->data;//5555
	pst_msg_reply->mid = nid;
	pst_msg_reply->result = reply_result;
	pst_msg->sync_word_heb = SYNC_WORD_H;
	pst_msg->sync_word_leb = SYNC_WORD_L;
	pst_msg->mid = MID_REPLY;
	pst_msg->size_leb = (uint8_t)reply_len;
	pst_msg->size_heb = (uint8_t)(reply_len>>8);
	#if UART_PRT
	printf("reply_len:%d pst_msg->size_leb:%02x pst_msg->size_hed:%02x\n", reply_len, pst_msg->size_leb, pst_msg->size_heb);
	#endif
	
	switch(nid)
	{
		case MID_RESET:
			printf("MID_RESET\n");
			break;

		case MID_GETSTATUS:
		{
			printf("MID_GETSTATUS\n");
			s_msg_reply_getstatus_data *pst_getstatus_data = (s_msg_reply_getstatus_data*)pst_msg_reply->data;
			pst_getstatus_data->status = g_device_static;
			break;
		}
		case MID_VERIFY:
		{
			printf("MID_VERIFY\n");
			if (reply_result == MR_SUCCESS)
			{
				s_msg_reply_verify_data *pst_verify = (s_msg_reply_verify_data*)pst_msg_reply->data;
				if (g_demo_mode)
				{
					pst_verify->user_id_leb = 0xff;
					pst_verify->user_id_heb = 0xff;
				}
				else
				{
					pst_verify->user_id_leb = (uint8_t)usr_pass_id_get();
					pst_verify->user_id_heb = (uint8_t)(usr_pass_id_get()>>8);
				}
			
				pst_verify->admin = 0;
				//zcy md 0 to 200 for fix hand proc
				pst_verify->unlockStatus = 200;
			        //zcy del231023
				//memcpy(pst_verify->user_name, "user", 5);
			}
			
			g_verify_timeout = 0;
			g_device_static = MS_STANDBY;
				
			//if (g_demo_mode == SIGN_ENABLE)
			//g_demo_mode = SIGN_DISABLE;

			break;
		}
		case MID_ENROLL:
		{
			printf("MID_ENROLL\n");

			if (reply_result == MR_SUCCESS)
			{
				s_msg_reply_enroll_data *pst_enroll = (s_msg_reply_enroll_data*)pst_msg_reply->data;
				pst_enroll->user_id_leb = (uint8_t)enroll_usr_id_get();
				pst_enroll->user_id_heb = (uint8_t)(enroll_usr_id_get() >> 8);
				pst_enroll->face_direction = g_five_input_face_direction;
			}

			break;
		}
		case MID_ENROLL_SINGLE:
		{
			printf("MID_ENROLL_SINGLE\n");

			if (reply_result == MR_SUCCESS)
			{
				s_msg_reply_enroll_data *pst_enroll = (s_msg_reply_enroll_data*)pst_msg_reply->data;
				pst_enroll->user_id_leb = (uint8_t)signle_enroll_usr_id_get();
				pst_enroll->user_id_heb = (uint8_t)(signle_enroll_usr_id_get() >> 8);
				pst_enroll->face_direction = FACE_DIRECTION_MIDDLE;//zcy md FACE_STATE_NORMAL to 
			}
			break;
		}
		case MID_ENROLL_ITG:
		{
			printf("MID_ENROLL_ITG\n");

			if (reply_result == MR_SUCCESS)
			{
				s_msg_reply_enroll_data *pst_enroll = (s_msg_reply_enroll_data*)pst_msg_reply->data;
				if (g_itg_enroll_type == FACE_ENROLL_TYPE_INTERACTIVE)
				{
					pst_enroll->user_id_leb = (uint8_t)enroll_usr_id_get();
					pst_enroll->user_id_heb = (uint8_t)(enroll_usr_id_get() >> 8);
				}
				else
				{	
					pst_enroll->user_id_leb = (uint8_t)signle_enroll_usr_id_get();
					pst_enroll->user_id_heb = (uint8_t)(signle_enroll_usr_id_get() >> 8);
				}
				pst_enroll->face_direction = g_five_input_face_direction;
				//printf("pst_enroll->user_id_leb:%d\n", pst_enroll->user_id_leb);
			}

			break;
		}
		case MID_SNAPIMAGE:
			printf("MID_SNAPIMAGE\n");
			break;

		case MID_GETSAVEDIMAGE:
		{
			printf("MID_GETSAVEDIMAGE\n");
			if (reply_result == MR_SUCCESS)
			{
				s_msg_reply_get_saved_image_data *pst_get_saved_image = (s_msg_reply_get_saved_image_data*)pst_msg_reply->data;
				pst_get_saved_image->image_size[3] = (uint8_t)g_send_image_buf_size;
				pst_get_saved_image->image_size[2] = (uint8_t)(g_send_image_buf_size>>8);
				pst_get_saved_image->image_size[1] = (uint8_t)(g_send_image_buf_size>>16);
				pst_get_saved_image->image_size[0] = (uint8_t)(g_send_image_buf_size>>24);
			}
			break;
		}
		case MID_UPLOADIMAGE:
			//printf("MID_UPLOADIMAGE\n");

			if (reply_result == MR_SUCCESS)
			{
				if (st_image_date_send(pst_msg_reply->data))
					MY_ERR_PRT("st_image_date_send", MY_ERRROR);					
			}
			break;

		case MID_DELUSER:
			printf("MID_DELUSER\n");
			break;
			
		case MID_DELALL:
			printf("MID_DELALL\n");
			break;

		case MID_FACERESET:
			printf("MID_FACERESET\n");
			break;

		case MID_GET_ALL_USERID:
		{
			printf("MID_GET_ALL_USERID\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			s_msg_reply_all_userid_data *pst_all_userid_data = (s_msg_reply_all_userid_data*)pst_msg_reply->data;
			if (pst_usr_list_info->usr_num == 0xff)
				pst_all_userid_data->user_counts = 0;
			else
			{
				pst_all_userid_data->user_counts = pst_usr_list_info->usr_num;
				uint8_t i, j = 0;
				for (i = 0; i < MAX_USER_COUNTS; i++)
				{
					if (pst_usr_list_info->sub_usr_list[i].usr_sign == 1)
					{	
						pst_all_userid_data->users_id[j * 2] = (uint8_t)0;
						pst_all_userid_data->users_id[j * 2 + 1] = (uint8_t)(i + 1);

						if ((++j) >=  pst_all_userid_data->user_counts)
							break;
					}
				}
			}
			break;
		}
		case MID_GET_VERSION:
		{
			printf("MID_GET_VERSION\n");
			s_msg_reply_version_data *pst_version_data = (s_msg_reply_version_data*)pst_msg_reply->data;
			memcpy(pst_version_data->version_info, g_version, sizeof(g_version));
			//memcpy(pst_version_data->version_info, FIRMWARE_VERSION, sizeof(FIRMWARE_VERSION));
			break;
		}
		case MID_GET_VERSION_ZA:
		{
			printf("MID_GET_VERSION_ZA\n");
			s_msg_reply_version_data *pst_version_data = (s_msg_reply_version_data*)pst_msg_reply->data;
			memcpy(pst_version_data->version_info, g_version_za, sizeof(g_version_za));
			//memcpy(pst_version_data->version_info, FIRMWARE_VERSION_ZA, sizeof(FIRMWARE_VERSION_ZA));
			break;
		}
		case MID_GET_VERSION_ZAPRD:
		{
			printf("MID_GET_VERSION_ZAPRD\n");
			s_msg_reply_version_data *pst_version_data = (s_msg_reply_version_data*)pst_msg_reply->data;
			//memcpy(pst_version_data->version_info, g_version_za, sizeof(g_version_za));
			memcpy(pst_version_data->version_info, FIRMWARE_VERSION_ZAPRD, sizeof(FIRMWARE_VERSION_ZAPRD));
			break;
		}
		case MID_START_OTA:
			printf("MID_START_OTA\n");
			break;

		case MID_INIT_ENCRYPTION:
		{
			printf("MID_INIT_ENCRYPTION\n");
			s_msg_reply_init_encryption_data *pst_init_encryption_data = (s_msg_reply_version_data*)pst_msg_reply->data;
			memcpy(pst_init_encryption_data->device_id, "ZF50S061101240789321", 20);//zcy note use fix data
			break;
		}
		case MID_CONFIG_BAUDRATE:
			printf("MID_CONFIG_BAUDRATE\n");
			break;

		case MID_SET_RELEASE_ENC_KEY:
			printf("MID_SET_RELEASE_ENC_KEY\n");
			break;

		case MID_SET_DEBUG_ENC_KEY:
			printf("MID_SET_DEBUG_ENC_KEY\n");
			break;	
			
		case MID_OTA_HEADER:
			printf("MID_OTA_HEADER\n");
			break;	

		case MID_GET_OTA_STATUS:
		{
			printf("MID_GET_OTA_STATUS\n");
			s_msg_reply_getotastatus_data *pst_getotastatus_data = (s_msg_reply_getotastatus_data*)pst_msg_reply->data;
			if (reply_result == MR_SUCCESS)
				pst_getotastatus_data->ota_status = 4;
			else
				pst_getotastatus_data->ota_status = 0;
			pst_getotastatus_data->next_pid_e[0] = (uint8_t)0;
			pst_getotastatus_data->next_pid_e[1] = (uint8_t)0;
			break;	
		}
		case MID_OTA_PACKET:
			//printf("MID_OTA_PACKET\n");
			break;

		case MID_SET_UVC_NET_FLAG:
			printf("MID_SET_UVC_NET_FLAG\n");
			break;

		case MID_SET_THRESHOLD_LEVEL:
			printf("MID_SET_THRESHOLD_LEVEL\n");
			break;

		case MID_POWERDOWN:	
			printf("MID_POWERDOWN\n");
			break;

		case MID_DEMOMODE:
			printf("MID_DEMOMODE\n");
			break;

		case MID_GET_INIT_TIME:
		{
			printf("MID_GET_INIT_TIME\n");
			unsigned short ready_time = (unsigned short)((g_ready_pts - g_start_pts + FORE_TIME) / 1000);
			//printf("ready_time:%d  %d\n", ready_time, g_ready_pts - g_start_pts + FORE_TIME);
			s_msg_reply_init_time *pst_get_init_time= (s_msg_reply_init_time*)pst_msg_reply->data;
			pst_get_init_time->time[0] = (uint8_t)(ready_time>>8);
			pst_get_init_time->time[1] = (uint8_t)ready_time;
			break;
		}
		default:
			MY_ERR_PRT("public_mid_reply not support nid", nid);
			break;
	}

	/* 掉电不发消息 */
	unsigned char *p_face_power_sign = face_power_sign();
	if (*p_face_power_sign == SIGN_DISABLE)
	{
		MY_ERR_PRT("lock_ia_sign", *p_face_power_sign);
		free(pst_msg);
		pst_msg = NULL;	
		return 0;
	}

	int i;
	#if UART_PRT  //no check sun print
	printf("g_enc_mode %d \n",g_enc_mode);
	   printf("msg_len %d \n",msg_len);
	   printf("tx start  \n");
		for (i=0; i<msg_len-1; i++)
		{
			printf("%02x ", ((uint8_t*)pst_msg)[i]);	
		}
		 printf("\n tx end \n");
	#endif
	
	if (g_enc_mode == 0x2)
	{
		unsigned int enc_data_size = reply_len + 3;
		
		enc_bytes(&pst_msg->mid, enc_data_size, g_enc_key, 16, &pst_msg->mid);
		
		msg_len+=2;
		s_enc_msg *pst_enc_msg = (s_enc_msg*)malloc(msg_len);
		memset(pst_enc_msg, 0, msg_len);
		pst_enc_msg->sync_word_heb = SYNC_WORD_H;
		pst_enc_msg->sync_word_leb = SYNC_WORD_L;
		pst_enc_msg->size_leb = (uint8_t)enc_data_size;
		pst_enc_msg->size_heb = (uint8_t)(enc_data_size>>8);
		memcpy(&pst_enc_msg->data, &pst_msg->mid, enc_data_size);
		
		
		
		//printf("%02x %02x %02x %02x ", ((uint8_t*)pst_enc_msg)[0], ((uint8_t*)pst_enc_msg)[1], ((uint8_t*)pst_enc_msg)[2], ((uint8_t*)pst_enc_msg)[3]);
		/* parity_check 加密校验码从data开始计算 */
		for (i=4; i<msg_len-1; i++)
		{
			((uint8_t*)pst_enc_msg)[msg_len-1] ^=((uint8_t*)pst_enc_msg)[i];
			//printf("%02x ", ((uint8_t*)pst_enc_msg)[i]);
		}
		//printf("%02x\n", ((uint8_t*)pst_enc_msg)[msg_len-1]);
		
	#if UART_PRT  //no check sun print
	   	printf("enc data \n");
	   	printf("tx start  \n");
		for (i=0; i<msg_len-1; i++)
		{
			printf("%02x ", ((uint8_t*)pst_enc_msg)[i]);	
		}
		 printf("\n tx end \n");
	#endif

		ret = rt_device_write(serial, 0, (uint8_t*)pst_enc_msg, msg_len);
		if (ret != msg_len)
		{
			MY_ERR_PRT("rt_device_write pst_enc_msg", ret);
			return MY_ERRROR;
		}

		free(pst_enc_msg);
		pst_enc_msg = NULL;	
	}
	else if (g_enc_mode > 0x0){//aes128 zcy add
	      unsigned int enc_data_size = reply_len + 3;
	     
		enc_data_size=DSM_CheckBodySize(enc_data_size);
		
		//printf("enc_data_size=%02x\n", enc_data_size);
		
		aesEncrypt(g_enc_key, 16, &pst_msg->mid, &pst_msg->mid, enc_data_size);
		msg_len = enc_data_size + 5;
		
		s_enc_msg *pst_enc_msg = (s_enc_msg*)rt_malloc(msg_len);
		memset(pst_enc_msg, 0, msg_len);
		pst_enc_msg->sync_word_heb = SYNC_WORD_H;
		pst_enc_msg->sync_word_leb = SYNC_WORD_L;
		pst_enc_msg->size_leb = (uint8_t)enc_data_size;
		pst_enc_msg->size_heb = (uint8_t)(enc_data_size>>8);
		memcpy(&pst_enc_msg->data, &pst_msg->mid, enc_data_size);
		
		//printf("%02x %02x %02x %02x ", ((uint8_t*)pst_enc_msg)[0], ((uint8_t*)pst_enc_msg)[1], ((uint8_t*)pst_enc_msg)[2], ((uint8_t*)pst_enc_msg)[3]);
		/* parity_check 加密校验码从data开始计算 */
		for (i=4; i<msg_len-1; i++)
		{
			((uint8_t*)pst_enc_msg)[msg_len-1] ^=((uint8_t*)pst_enc_msg)[i];
		}
		//printf("%02x\n", ((uint8_t*)pst_enc_msg)[msg_len-1]);
		
	#if UART_PRT  //no check sun print
	   	printf("11 enc data \n");
	   	printf("tx start  \n");
		for (i=0; i<msg_len; i++)
		{
			printf("%02x ", ((uint8_t*)pst_enc_msg)[i]);	
		}
		 printf("\n tx end \n");
	#endif
	
		ret = rt_device_write(serial, 0, (uint8_t*)pst_enc_msg, msg_len);
		if (ret != msg_len)
		{
			MY_ERR_PRT("rt_device_write pst_enc_msg", ret);
			rt_free(pst_enc_msg);
			pst_enc_msg = NULL;	
			return MY_ERRROR;
		}

		rt_free(pst_enc_msg);
		pst_enc_msg = NULL;	
	}
	else
	{
		//parity_check
		//printf("%02x %02x ", ((uint8_t*)pst_msg)[0], ((uint8_t*)pst_msg)[1]);
		
		for (i=2; i<msg_len-1; i++)
		{
			((uint8_t*)pst_msg)[msg_len-1] ^=((uint8_t*)pst_msg)[i];
			//printf("%02x ", ((uint8_t*)pst_msg)[i]);
		}
		//printf("%02x\n", ((uint8_t*)pst_msg)[msg_len-1]);

		ret = rt_device_write(serial, 0, (uint8_t*)pst_msg, msg_len);
		if (ret != msg_len) 
		{
			MY_ERR_PRT("rt_device_write pst_msg", ret);
			return MY_ERRROR;
		}
	}

	free(pst_msg);
	pst_msg = NULL;	

	return 0;
}

/************************************
	Function: mod_to_host_note
*************************************/
static int st_mod_to_host_note(uint8_t nid, uint8_t nid_result)
{
	int ret;
	unsigned int note_len;

	if (nid == NID_FACE_STATE)
		note_len = sizeof(s_msg_note_data) + sizeof(s_note_data_face);
	else if (nid == NID_OTA_DONE)
		note_len = sizeof(s_msg_note_data) + 1;
	else
		note_len = sizeof(s_msg_note_data) + 0;

	unsigned int msg_len = sizeof(s_msg) + note_len;
	//zcy add 
	unsigned int msg_buf_len = msg_len + 64;//zcy andd for test
	s_msg *pst_msg = (s_msg*)malloc(msg_buf_len);
	memset(pst_msg, 0, msg_buf_len);
	
	s_msg_note_data *pst_msg_note = (s_msg_note_data*)pst_msg->data;
	
	pst_msg->sync_word_heb = SYNC_WORD_H;
	pst_msg->sync_word_leb = SYNC_WORD_L;
	pst_msg->mid = MID_NOTE;
	pst_msg->size_leb = (uint8_t)note_len;
	pst_msg->size_heb = (uint8_t)(note_len>>8);
	pst_msg_note->nid = nid;

	switch (nid)
	{
		case NID_READY:
			printf("NID_READY\n");

			break;

		case NID_FACE_STATE:
		{
			printf("NID_FACE_STATE\n");
			s_note_data_face *pst_note_face = (s_note_data_face*)pst_msg_note->data;
			memset(pst_note_face, 0, sizeof(s_note_data_face));
			pst_note_face->state = nid_result;
			break;
		}
		case NID_UNKNOWNERROR: 		
			printf("NID_UNKNOWNERROR\n");
			
			break;

		case NID_OTA_DONE:
			pst_msg_note->data[0] = nid_result;
			printf("NID_OTA_DONE\n");
			break;

		case NID_EYE_STATE:
			printf("NID_EYE_STATE\n");
			break;

		case NID_AUTHORIZATION:
			printf("NID_AUTHORIZATION\n");
			break;
		default:
			MY_ERR_PRT("public_mid_note not support nid", nid);
			return -1;	
	}

	/* 掉电不发消息 */
	unsigned char *p_face_power_sign = face_power_sign();
	if (*p_face_power_sign == SIGN_DISABLE)
	{
		MY_ERR_PRT("lock_ia_sign", *p_face_power_sign);
		free(pst_msg);
		pst_msg = NULL;	
		return 0;
	}

	int i;
	#if UART_PRT
		for (i=0; i<msg_len-1; i++)
		{
			printf("%02x ", ((uint8_t*)pst_msg)[i]);
		}
		
	#endif
	if (g_enc_mode == 0x2)
	{
		unsigned int enc_data_size = note_len + 3;
		
		//printf("enc_data_size %d \n", enc_data_size);
		enc_bytes(&pst_msg->mid, enc_data_size, g_enc_key, 16, &pst_msg->mid);
		
		msg_len+=2;
		s_enc_msg *pst_enc_msg = (s_enc_msg*)malloc(msg_len);
		memset(pst_enc_msg, 0, msg_len);
		pst_enc_msg->sync_word_heb = SYNC_WORD_H;
		pst_enc_msg->sync_word_leb = SYNC_WORD_L;
		pst_enc_msg->size_leb = (uint8_t)enc_data_size;
		pst_enc_msg->size_heb = (uint8_t)(enc_data_size>>8);
		memcpy(&pst_enc_msg->data, &pst_msg->mid, enc_data_size);

		//printf("%02x %02x %02x %02x ", ((uint8_t*)pst_enc_msg)[0], ((uint8_t*)pst_enc_msg)[1], ((uint8_t*)pst_enc_msg)[2], ((uint8_t*)pst_enc_msg)[3]);
		
		/* parity_check 加密校验码从data开始计算 */
		for (i=4; i<msg_len-1; i++)
		{
			
			((uint8_t*)pst_enc_msg)[msg_len-1] ^=((uint8_t*)pst_enc_msg)[i];
			
		}
		//printf("%02x\n", ((uint8_t*)pst_enc_msg)[msg_len-1]);

		ret = rt_device_write(serial, 0, (uint8_t*)pst_enc_msg, msg_len);
		if (ret != msg_len)
		{
			MY_ERR_PRT("rt_device_write pst_enc_msg", ret);
			return MY_ERRROR;
		}

		free(pst_enc_msg);
		pst_enc_msg = NULL;
	}
	else if(g_enc_mode > 0x0){//zcy add
	
		unsigned int enc_data_size = note_len + 3;
		
		enc_data_size=DSM_CheckBodySize(enc_data_size);
		
		//printf("enc_data_size %d \n", enc_data_size);
		
		aesEncrypt(g_enc_key, 16, &pst_msg->mid, &pst_msg->mid, enc_data_size);
		
		msg_len = enc_data_size + 5;
		s_enc_msg *pst_enc_msg = (s_enc_msg*)malloc(msg_len);
		memset(pst_enc_msg, 0, msg_len);
		pst_enc_msg->sync_word_heb = SYNC_WORD_H;
		pst_enc_msg->sync_word_leb = SYNC_WORD_L;
		pst_enc_msg->size_leb = (uint8_t)enc_data_size;
		pst_enc_msg->size_heb = (uint8_t)(enc_data_size>>8);
		memcpy(&pst_enc_msg->data, &pst_msg->mid, enc_data_size);

		//printf("%02x %02x %02x %02x ", ((uint8_t*)pst_enc_msg)[0], ((uint8_t*)pst_enc_msg)[1], ((uint8_t*)pst_enc_msg)[2], ((uint8_t*)pst_enc_msg)[3]);
		
		/* parity_check 加密校验码从data开始计算 */
		for (i=4; i<msg_len-1; i++)
		{
			((uint8_t*)pst_enc_msg)[msg_len-1] ^=((uint8_t*)pst_enc_msg)[i];
			
		}
		//printf("%02x\n", ((uint8_t*)pst_enc_msg)[msg_len-1]);

		ret = rt_device_write(serial, 0, (uint8_t*)pst_enc_msg, msg_len);
		if (ret != msg_len)
		{
			MY_ERR_PRT("rt_device_write pst_enc_msg", ret);
			free(pst_enc_msg);
			pst_enc_msg = NULL;
			return MY_ERRROR;
		}

		free(pst_enc_msg);
		pst_enc_msg = NULL;	
		
	}
	else
	{
		//printf("msg_len== %d \n", msg_len);
		//parity_check
		for (i=2; i<msg_len-1; i++)
		{
			((uint8_t*)pst_msg)[msg_len-1] ^=((uint8_t*)pst_msg)[i];
			//zcy md prt
			//printf("%02x ", ((uint8_t*)pst_msg)[i]);
		}
		
		ret = rt_device_write(serial, 0, (uint8_t*)pst_msg, msg_len);
		if (ret != msg_len) {
			MY_ERR_PRT("rt_device_write", ret);
			return MY_ERRROR;
		}	
	}	

	free(pst_msg);
	pst_msg = NULL; 
	
	return 0;
}

/* not use */
static int st_mod_to_host_image(uint8_t nid)
{
	unsigned int msg_len = sizeof(s_msg);
	s_msg *pst_msg = (s_msg*)malloc(msg_len);
	memset(pst_msg, 0, msg_len);
	pst_msg->mid = MID_IMAGE;
	pst_msg->sync_word_heb = SYNC_WORD_H;
	pst_msg->sync_word_leb = SYNC_WORD_L;
	//pst_msg->size_leb = (uint8_t)image_len;
	//pst_msg->size_heb = (uint8_t)(image_len>>8);

	int i;
	//parity_check
	for (i=2; i<msg_len-1; i++)
		((uint8_t*)pst_msg)[msg_len-1] ^=((uint8_t*)pst_msg)[i];

	free(pst_msg);
	pst_msg = NULL;		

	return 0;
}

/****************************************
	Function: mod_to_host_note
	note::reply_result: 可用做人脸状态zcy
*****************************************/
int st_mod_to_host_mid(uint8_t mid, uint8_t nid, uint8_t reply_result)
{
	int ret;
	switch (mid)
	{
		case MID_REPLY:
			//printf("mod_to_host MID_REPLY\n");
			ret = st_mod_to_host_reply(nid, reply_result);//55555
			if (ret != 0)
			{
				MY_ERR_PRT("public_mid_reply", ret);
				return MY_ERRROR;
			}
			break;

		case MID_NOTE:
			printf("mod_to_host MID_NOTE\n");

			ret = st_mod_to_host_note(nid, reply_result);
			if (ret != 0) 
			{
				MY_ERR_PRT("public_mid_note", ret);
				return MY_ERRROR;
			}
			break;

		case MID_IMAGE: 
			printf("mod_to_host MID_IMAGE\n");
			ret = st_mod_to_host_image(nid);
			if (ret != 0) 
			{
				MY_ERR_PRT("public_mid_image", ret);
				return MY_ERRROR;
			}
			break;

		default:
			MY_ERR_PRT("public_mod_to_host_mid not support id", -1);
			return -1;
	}

	return 0;
}

static int st_host_to_mod_mid(s_msg *pst_mod_recv_msg)
{
	int ret = 0;
	switch(pst_mod_recv_msg->mid)
	{
		case MID_RESET:
			printf("recv MID_RESET\n");

			/* ota 暂不能停止 */
			if (g_ota_mode)
			{
				MY_ERR_PRT("ota can not reset", g_ota_mode);
				if (st_mod_to_host_mid(MID_REPLY, MID_RESET, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				break;
			}

			g_verify_timeout = 0;
			g_enroll_timeout = 0;
			g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;
			enroll_status_reset();

			if (st_mod_to_host_mid(MID_REPLY, MID_RESET, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			g_device_static = MS_STANDBY;
			if (g_demo_mode == SIGN_ENABLE)
				g_demo_mode = SIGN_DISABLE;

			break;

		case MID_GETSTATUS:
			printf("recv MID_GETSTATUS\n");
			if (st_mod_to_host_mid(MID_REPLY, MID_GETSTATUS, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;

		case MID_VERIFY:
		{
			printf("recv MID_VERIFY\n");
			s_msg_verify_data *pst_verify = (s_msg_verify_data*)pst_mod_recv_msg->data;
			if (pst_verify->timeout < 3)//zcy md 0 to 4
				g_verify_timeout = 5;
			else
				g_verify_timeout = pst_verify->timeout;

			g_device_static = MS_BUSY;
			printf("recv MID_VERIFY %d\n",g_verify_timeout);
			break;
		}
		case MID_ENROLL:
		{
			printf("recv MID_ENROLL\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			if (pst_usr_list_info->usr_num >= MAX_USER_COUNTS && pst_usr_list_info->usr_num != 0xff)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_FAILED4_MAXUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				break;	
			}

			s_msg_enroll_data *pst_enroll = (s_msg_enroll_data*)pst_mod_recv_msg->data;
			if (pst_enroll->timeout < 3)
				g_enroll_timeout = 10;
			else
				g_enroll_timeout = pst_enroll->timeout;
			printf("pst_enroll->admin : %d\n", pst_enroll->admin);
			printf("direction : %d\n", pst_enroll->face_direction);
#ifdef DEFAULT_DUPLICATE_FACE_LIMIT
			if (g_five_input_face_direction == FACE_DIRECTION_UNDEFINE)
			{
				if (duplicate_face_judge(0) == 0)
				{

			/* 五点人脸录入 */
			if (five_point_enroll(pst_enroll) < 0)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_FAILED4_TIMEOUT) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

				MY_ERR_PRT("five_point_enroll", MY_ERRROR);
				break;
			}

			g_enroll_timeout = 0;
			g_five_input_face_direction |= pst_enroll->face_direction;
			
					/* 录入成功后回复主控 */
					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_SUCCESS) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

					if (g_five_input_face_direction == (FACE_DIRECTION_UP | FACE_DIRECTION_DOWN | FACE_DIRECTION_LEFT | FACE_DIRECTION_RIGHT | FACE_DIRECTION_MIDDLE))
						g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;
				}
				else
				{
					g_enroll_timeout = 0;

					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_FAILED4_FACEENROLLED) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				}
			}
			else
			{
#endif
				/* 五点人脸录入 */
				if (five_point_enroll(pst_enroll) < 0)
				{
					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_FAILED4_TIMEOUT) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

					MY_ERR_PRT("five_point_enroll", MY_ERRROR);
					break;
				}

				g_enroll_timeout = 0;
				g_five_input_face_direction |= pst_enroll->face_direction;

				/* 录入成功后回复主控 */
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

				if (g_five_input_face_direction == (FACE_DIRECTION_UP | FACE_DIRECTION_DOWN | FACE_DIRECTION_LEFT | FACE_DIRECTION_RIGHT | FACE_DIRECTION_MIDDLE))
					g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;

#ifdef DEFAULT_DUPLICATE_FACE_LIMIT
			}
#endif

			break;
		}
		case MID_ENROLL_SINGLE:
		{
			printf("recv MID_ENROLL_SINGLE\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			if (pst_usr_list_info->usr_num >= MAX_USER_COUNTS && pst_usr_list_info->usr_num != 0xff)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_SINGLE, MR_FAILED4_MAXUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				break;	
			}

			s_msg_enroll_data *pst_enroll = (s_msg_enroll_data*)pst_mod_recv_msg->data;
			if (pst_enroll->timeout < 3)
				g_enroll_timeout = 10;
			else
				g_enroll_timeout = pst_enroll->timeout;				

#ifdef DEFAULT_DUPLICATE_FACE_LIMIT
			if (duplicate_face_judge(0) == 0)
			{
#endif
			/* 单张人脸录入 */
			if (single_enroll(pst_enroll) < 0)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_SINGLE, MR_FAILED4_TIMEOUT) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				MY_ERR_PRT("single_enroll", MY_ERRROR);
				break;
			}
			
			g_enroll_timeout = 0;

			/* 录入成功后回复主控 */
			if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_SINGLE, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
#ifdef DEFAULT_DUPLICATE_FACE_LIMIT
			}
			else
			{
				g_enroll_timeout = 0;

				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_SINGLE, MR_FAILED4_FACEENROLLED) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
#endif
			break;
		}
		case MID_ENROLL_ITG:
		{
			printf("recv MID_ENROLL_ITG\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			if (pst_usr_list_info->usr_num >= MAX_USER_COUNTS && pst_usr_list_info->usr_num != 0xff)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_MAXUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				break;	
			}

			s_msg_enroll_itg *pst_enroll_itg = (s_msg_enroll_itg*)pst_mod_recv_msg->data;
			g_enroll_timeout = pst_enroll_itg->timeout;
				//zcy add
			if(g_enroll_timeout < 3)
				g_enroll_timeout = 10;
				
			g_itg_enroll_type = pst_enroll_itg->enroll_type;

			if (g_five_input_face_direction == FACE_DIRECTION_UNDEFINE)
			{
				if (duplicate_face_judge(pst_enroll_itg->enable_duplicate) == 0)
				{
					if (pst_enroll_itg->enroll_type == FACE_ENROLL_TYPE_INTERACTIVE)
					{
						/* 五点人脸录入 */
						if (five_point_enroll((s_msg_enroll_data *)pst_enroll_itg) < 0) 
						{
							if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_TIMEOUT) != 0)
								MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

						MY_ERR_PRT("five_point_enroll", MY_ERRROR);
						break;
					}

				g_enroll_timeout = 0;
				g_five_input_face_direction |= pst_enroll_itg->face_direction;

				/* 录入成功后回复主控 */
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

				if (g_five_input_face_direction == (FACE_DIRECTION_UP | FACE_DIRECTION_DOWN | FACE_DIRECTION_LEFT | FACE_DIRECTION_RIGHT | FACE_DIRECTION_MIDDLE))
					g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;

				break;	
			}
			else if (pst_enroll_itg->enroll_type == FACE_ENROLL_TYPE_SINGLE)
			{
				
				/* 单张人脸录入 */
				if (single_enroll((s_msg_enroll_data *)pst_enroll_itg) < 0) 
				{
					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_TIMEOUT) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
					MY_ERR_PRT("single_enroll", MY_ERRROR);
					break;
				}

				g_enroll_timeout = 0;

				/* 录入成功后回复主控 */
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_INVALIDPARAM) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				}
			}
			else
			{
				g_enroll_timeout = 0;

					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_FACEENROLLED) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				}
			}
			else
			{
				if (pst_enroll_itg->enroll_type == FACE_ENROLL_TYPE_INTERACTIVE)
				{
					/* 五点人脸录入 */
					if (five_point_enroll((s_msg_enroll_data *)pst_enroll_itg) < 0) 
					{
						if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_TIMEOUT) != 0)
							MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

						MY_ERR_PRT("five_point_enroll", MY_ERRROR);
						break;
					}

					g_enroll_timeout = 0;
					g_five_input_face_direction |= pst_enroll_itg->face_direction;

					/* 录入成功后回复主控 */
					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_SUCCESS) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

					if (g_five_input_face_direction == (FACE_DIRECTION_UP | FACE_DIRECTION_DOWN | FACE_DIRECTION_LEFT | FACE_DIRECTION_RIGHT | FACE_DIRECTION_MIDDLE))
						g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;

					break;	
				}
				else if (pst_enroll_itg->enroll_type == FACE_ENROLL_TYPE_SINGLE)
				{
					if (duplicate_face_judge(pst_enroll_itg->enable_duplicate) == 0)
					{
						/* 单张人脸录入 */
						if (single_enroll((s_msg_enroll_data *)pst_enroll_itg) < 0) 
						{
							if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_TIMEOUT) != 0)
								MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
							MY_ERR_PRT("single_enroll", MY_ERRROR);
							break;
						}

						g_enroll_timeout = 0;

						/* 录入成功后回复主控 */
						if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_SUCCESS) != 0)
							MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
					}
					else
					{
						g_enroll_timeout = 0;

						if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_FACEENROLLED) != 0)
							MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
					}
				}
				else
				{
					if (st_mod_to_host_mid(MID_REPLY, MID_ENROLL_ITG, MR_FAILED4_INVALIDPARAM) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				}
			}
			
			break;
		}
		case MID_SNAPIMAGE:
		{
			s_msg_snap_image_data *pst_snap_image_data = (s_msg_snap_image_data*)pst_mod_recv_msg->data;		
			printf("image_counts:%d  start_number:%d \n", pst_snap_image_data->image_counts, pst_snap_image_data->start_number);

			if (pst_snap_image_data->image_counts == 2 && pst_snap_image_data->start_number == 1)
			{
				g_cap_sign = SIGN_ENABLE;
				if (st_mod_to_host_mid(MID_REPLY, MID_SNAPIMAGE, MR_SUCCESS))
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{
				/* 其他数量暂不支持 */
				if (st_mod_to_host_mid(MID_REPLY, MID_SNAPIMAGE, MR_FAILED4_INVALIDPARAM))
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}

			break;
		}
		case MID_GETSAVEDIMAGE:
		{
			s_msg_get_saved_image_data *pst_get_saved_image = (s_msg_get_saved_image_data*)pst_mod_recv_msg->data;
			if (st_get_jpg_save(pst_get_saved_image->image_number, &gstPicSize, &g_send_image_buf_size, &gp_send_image_buf))
				MY_ERR_PRT("st_get_jpg_save", MY_ERRROR);

			/* 回复 */
			if (st_mod_to_host_mid(MID_REPLY, MID_GETSAVEDIMAGE, MR_SUCCESS))
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			break;
		}
		case MID_UPLOADIMAGE:
		{
			s_msg_upload_image_data *pst_upload_image_data = (s_msg_upload_image_data*)pst_mod_recv_msg->data;
			 g_upload_image_offset = (unsigned int)pst_upload_image_data->upload_image_offset[0]<<24 | (unsigned int)pst_upload_image_data->upload_image_offset[1]<<16
									| (unsigned int)pst_upload_image_data->upload_image_offset[2]<<8 | (unsigned int)pst_upload_image_data->upload_image_offset[3];
			//printf("recv upload_image_offset[0][1][2][3]:%02x %02x %02x %02x\n", pst_upload_image_data->upload_image_offset[0], pst_upload_image_data->upload_image_offset[1], pst_upload_image_data->upload_image_offset[2], pst_upload_image_data->upload_image_offset[3]);						
			
			g_upload_image_size = (unsigned int)pst_upload_image_data->upload_image_size[0]<<24 | (unsigned int)pst_upload_image_data->upload_image_size[1]<<16
								 	| (unsigned int)pst_upload_image_data->upload_image_size[2]<<8 | (unsigned int)pst_upload_image_data->upload_image_size[3];
			//printf("recv upload_image_size[0][1][2][3]:%02x %02x %02x %02x\n", pst_upload_image_data->upload_image_size[0], pst_upload_image_data->upload_image_size[1], pst_upload_image_data->upload_image_size[2], pst_upload_image_data->upload_image_size[3]);

			if (g_upload_image_size + g_upload_image_offset > g_send_image_buf_size)
			{
				MY_ERR_PRT("error g_upload_image_size + g_upload_image_offset ", g_upload_image_size + g_upload_image_offset);
				if (st_mod_to_host_mid(MID_REPLY, MID_UPLOADIMAGE, MR_FAILED4_INVALIDPARAM))
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{
				/* 回复 */
				if (st_mod_to_host_mid(MID_REPLY, MID_UPLOADIMAGE, MR_SUCCESS))
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}

			break;
		}
		case MID_DELUSER:
		{
			printf("recv MID_DELUSER\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			s_msg_deluser_data *pst_deluser_data = (s_msg_deluser_data*)pst_mod_recv_msg->data;
			uint16_t delete_usr_id = (uint16_t)pst_deluser_data->user_id_leb | (uint16_t)pst_deluser_data->user_id_heb << 8;
			printf("delete_usr_id:%d\n", delete_usr_id);

			if (delete_usr_id < 1 || pst_usr_list_info->usr_num == 0 || pst_usr_list_info->usr_num == 0xff || pst_usr_list_info->sub_usr_list[delete_usr_id-1].usr_sign == 0 || pst_usr_list_info->sub_usr_list[delete_usr_id-1].usr_sign == 0xff)
			{
				MY_ERR_PRT("delete_usr_id", delete_usr_id);
				if (st_mod_to_host_mid(MID_REPLY, MID_DELUSER, MR_FAILED4_UNKNOWNUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else if(delete_usr_id > MAX_USER_COUNTS)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_DELUSER, MR_FAILED4_MAXUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{	
				if (user_delete(delete_usr_id) != 0)
				{
					if (st_mod_to_host_mid(MID_REPLY, MID_DELUSER, MR_FAILED4_UNKNOWNREASON) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

					MY_ERR_PRT("user_delete", MY_ERRROR);				
					break;
				}
				else
				{
					if (st_mod_to_host_mid(MID_REPLY, MID_DELUSER, MR_SUCCESS) != 0)
						MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				}
			}

			break;
		}
		case MID_DELALL:
		{
			printf("recv MID_DELALL\n");

			/* 清空人脸 */
			if (usr_empty() < 0)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_DELALL, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

				MY_ERR_PRT("user_empty", MY_ERRROR);	
				break;
			}
			else
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_DELALL, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}

			break;
		}
		case MID_FACERESET:
			printf("recv MID_FACERESET\n");
			g_five_input_face_direction = FACE_DIRECTION_UNDEFINE;
			enroll_status_reset();

			if (st_mod_to_host_mid(MID_REPLY, MID_FACERESET, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;

		case MID_GET_ALL_USERID:
		{
			printf("recv MID_GET_ALL_USERID\n");
			USR_LIST_INFO_S *pst_usr_list_info = usr_list_info_get();
			if (pst_usr_list_info->usr_num == 0 || pst_usr_list_info->usr_num == 0xff)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_GET_ALL_USERID, MR_FAILED4_UNKNOWNUSER) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_GET_ALL_USERID, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			break;
		}	
		case MID_GET_VERSION:
		{
			printf("recv MID_GET_VERSION\n");
			if (st_mod_to_host_mid(MID_REPLY, MID_GET_VERSION, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;
		}
		case MID_GET_VERSION_ZA:
		{
			printf("recv MID_GET_VERSION_ZA\n");
			if (st_mod_to_host_mid(MID_REPLY, MID_GET_VERSION_ZA, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;
		}
		case MID_GET_VERSION_ZAPRD:
		{
			printf("recv MID_GET_VERSION_ZAPRD\n");
			if (st_mod_to_host_mid(MID_REPLY, MID_GET_VERSION_ZAPRD, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;
		}
		case MID_START_OTA:
		{
			s_msg_startota_data *pst_startota_data = (s_msg_startota_data*)pst_mod_recv_msg->data;
			g_ota_type = pst_startota_data->ota_type;
			if (pst_startota_data->ota_type < 4 || pst_startota_data->ota_type == 1)
			{	
				printf("g_ota_type:%d \n", g_ota_type);
				g_ota_mode = SIGN_ENABLE;
				while (g_ota_mode == SIGN_ENABLE)
					usleep(10*1000);

				/* 回复主控 */
				if (st_mod_to_host_mid(MID_REPLY, MID_START_OTA, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

				g_device_static = MS_OTA;
			}
			else
			{
				MY_ERR_PRT("pst_startota_data->ota_type", pst_startota_data->ota_type);
				if (st_mod_to_host_mid(MID_REPLY, MID_START_OTA, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}

			break;
		}
		case MID_INIT_ENCRYPTION:
		{
			printf("recv MID_INIT_ENCRYPTION\n");
			s_msg_init_encryption_data *pst_init_encryption_data = (s_msg_init_encryption_data*)pst_mod_recv_msg->data;
			printf("seed[%02x %02x %02x %02x]\n", pst_init_encryption_data->seed[0], pst_init_encryption_data->seed[1], pst_init_encryption_data->seed[2], pst_init_encryption_data->seed[3]);
				
			//if (pst_init_encryption_data->mode == 0x2)/* 异或取反加密模式 */
			{
				g_enc_mode = pst_init_encryption_data->mode;
				if(g_enc_mode != 0x02)
					g_enc_mode = 0x01;//aes
				

				if (!g_enc_mode_debug)
					key_rule_get(g_key_rule);

				/* genenckey */
				gen_enc_key(pst_init_encryption_data->seed, g_key_rule, g_enc_key);
				

				if (st_mod_to_host_mid(MID_REPLY, MID_INIT_ENCRYPTION, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			

			break;
		}
		case MID_CONFIG_BAUDRATE:
		{
			printf("recv MID_CONFIG_BAUDRATE\n");
			s_msg_config_baudrate *pst_config_baudrate = (s_msg_config_baudrate*)pst_mod_recv_msg->data;
			unsigned int baud_rate_selset[4] = {115200, 230400, 460800, 921600};

			if (st_mod_to_host_mid(MID_REPLY, MID_CONFIG_BAUDRATE, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			usleep(100*1000);
			if (uart_reset(SAMPLE_UART_NAME, baud_rate_selset[pst_config_baudrate->baudrate_index-1]) != 0)
			{
				MY_ERR_PRT("uart_reset", MY_ERRROR);
				if (st_mod_to_host_mid(MID_REPLY, MID_CONFIG_BAUDRATE, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				g_ota_mode = SIGN_DISABLE;
			}

			break;
		}
		case MID_SET_RELEASE_ENC_KEY:
		{
			/* 永久使用 */
			printf("recv MID_SET_RELEASE_ENC_KEY\n");
			s_msg_enc_key_number_data *pst_enc_key_number_data = (s_msg_enc_key_number_data*)pst_mod_recv_msg->data;
			
			/* 密钥抽取规则 */
			int i;
			for (i = 0; i < 16; i++)
				printf("%02x ", pst_enc_key_number_data->enc_key_number[i]);
			printf("\n");

			key_rule_store(pst_enc_key_number_data->enc_key_number);

			if (st_mod_to_host_mid(MID_REPLY, MID_SET_RELEASE_ENC_KEY, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			break;
		}
		case MID_SET_DEBUG_ENC_KEY:
		{
			/* 本次会话使用 */
			printf("recv MID_SET_DEBUG_ENC_KEY\n");
			s_msg_enc_key_number_data *pst_enc_key_number_data = (s_msg_enc_key_number_data*)pst_mod_recv_msg->data;
			g_enc_mode_debug = SIGN_ENABLE;
			
			/* 密钥抽取规则 */
			int i;
			for (i = 0; i < 16; i++)
				printf("%02x ", pst_enc_key_number_data->enc_key_number[i]);
			printf("\n");
			memcpy(g_key_rule, pst_enc_key_number_data->enc_key_number, KEY_SIZE);

			if (st_mod_to_host_mid(MID_REPLY, MID_SET_DEBUG_ENC_KEY, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			break;
		}
		case MID_OTA_HEADER:
		{
			printf("recv MID_OTA_HEADER\n");
			//printf("revc MID_OTA_HEADER\n");
			s_msg_otaheader_data *pst_otaheader_data = (s_msg_otaheader_data*)pst_mod_recv_msg->data;
			g_ota_sum_size = (unsigned int)pst_otaheader_data->fsize_b[0]<<24 | (unsigned int)pst_otaheader_data->fsize_b[1]<<16
									| (unsigned int)pst_otaheader_data->fsize_b[2]<<8 | (unsigned int)pst_otaheader_data->fsize_b[3];

			g_ota_sum_packet = (unsigned int)pst_otaheader_data->num_pkt[0]<<24 | (unsigned int)pst_otaheader_data->num_pkt[1]<<16
									| (unsigned int)pst_otaheader_data->num_pkt[2]<<8 | (unsigned int)pst_otaheader_data->num_pkt[3];

			g_ota_signle_psize = (uint16_t)pst_otaheader_data->pkt_size[0]<<8 | (uint16_t)pst_otaheader_data->pkt_size[1];

			printf("ota_sum_size :%d ota_sum_packet:%d ota_signle_psize:%d\n", g_ota_sum_size, g_ota_sum_packet, g_ota_signle_psize);
			/* md5校验暂时不用 */

			if (st_mod_to_host_mid(MID_REPLY, MID_OTA_HEADER, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);

			break;
		}
		case MID_GET_OTA_STATUS:
		{
			printf("recv MID_GET_OTA_STATUS\n");
			if (g_ota_mode == 2)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_GET_OTA_STATUS, MR_SUCCESS) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			}
			else
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_GET_OTA_STATUS, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				MY_ERR_PRT("g_ota_mode", g_ota_mode);
				g_ota_mode = SIGN_DISABLE;
			}
			break;
		}
		case MID_OTA_PACKET:
		{
			//printf("revc MID_OTA_PACKET\n");
			s_msg_otapacket_data *pst_otapacket_data = (s_msg_otapacket_data*)pst_mod_recv_msg->data;
			uint16_t packet_pid = (uint16_t)pst_otapacket_data->pid[0]<<8 | (uint16_t)pst_otapacket_data->pid[1];
			uint16_t psize = (uint16_t)pst_otapacket_data->psize[0]<<8 | (uint16_t)pst_otapacket_data->psize[1];
			if (ota_data_splice(packet_pid, psize, pst_otapacket_data->data) != 0)
			{
				MY_ERR_PRT("ota_data_splice", MY_ERRROR);
				if (st_mod_to_host_mid(MID_REPLY, MID_OTA_PACKET, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
				g_ota_mode = SIGN_DISABLE;
			}
			else
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_OTA_PACKET, MR_SUCCESS) != 0)
					MY_ERR_PRT("public_mod_to_host_mid", MY_ERRROR);

				if (packet_pid == g_ota_sum_packet - 1)
				{
					printf("-----------------revc ota ok----------------!\n");

					/* 数据接收完成 擦写flash */
					if (ota_write_falsh(gp_ota_data_puf, g_ota_sum_size, g_ota_type) != 0)
					{
						MY_ERR_PRT("ota_write_falsh", MY_ERRROR);
						
						//note
						if (st_mod_to_host_mid(MID_NOTE, NID_OTA_DONE, MR_FAILED4_UNKNOWNREASON) != 0) 
							MY_ERR_PRT("st_mod_to_host_mid", ret);

						return MY_ERRROR;
					}
					else
					{
						printf("ota_write_falsh ok!\n");
					}

					//note
					if (st_mod_to_host_mid(MID_NOTE, NID_OTA_DONE, MR_SUCCESS) != 0) 
					{
						MY_ERR_PRT("st_mod_to_host_mid", ret);
						return MY_ERRROR;
					}
					
					g_ota_mode = SIGN_DISABLE;
					g_device_static = MS_ERROR;
				}
			}
			break;
		}
		case MID_SET_UVC_NET_FLAG:
		{
			printf("revc MID_SET_UVC_NET_FLAG\n");
			unsigned int value = pst_mod_recv_msg->data[0];
	
			scan_mode_set(value);

			if (st_mod_to_host_mid(MID_REPLY, MID_SET_UVC_NET_FLAG, MR_SUCCESS) != 0)
				MY_ERR_PRT("public_mod_to_host_mid", MY_ERRROR);

			break;
		}
		case MID_SET_THRESHOLD_LEVEL:
			printf("revc MID_SET_THRESHOLD_LEVEL\n");
			s_msg_algo_threshold_level *pst_algo_threshold_level = (s_msg_enc_key_number_data*)pst_mod_recv_msg->data;
			printf("verify_threshold_level[%d] liveness_threshold_level[%d]\n",pst_algo_threshold_level->verify_threshold_level, pst_algo_threshold_level->liveness_threshold_level);
			if (pst_algo_threshold_level->verify_threshold_level < 0 || pst_algo_threshold_level->verify_threshold_level > 4 
				|| pst_algo_threshold_level->liveness_threshold_level < 0 || pst_algo_threshold_level->liveness_threshold_level > 4)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_SET_THRESHOLD_LEVEL, MR_FAILED4_INVALIDPARAM) != 0)
					MY_ERR_PRT("public_mod_to_host_mid", MY_ERRROR);
			}

			if (set_threshold_level(pst_algo_threshold_level->verify_threshold_level, pst_algo_threshold_level->liveness_threshold_level) != 0)
			{
				if (st_mod_to_host_mid(MID_REPLY, MID_SET_THRESHOLD_LEVEL, MR_FAILED4_UNKNOWNREASON) != 0)
					MY_ERR_PRT("public_mod_to_host_mid", MY_ERRROR);
			}

			if (st_mod_to_host_mid(MID_REPLY, MID_SET_THRESHOLD_LEVEL, MR_SUCCESS) != 0)
				MY_ERR_PRT("public_mod_to_host_mid", MY_ERRROR);
			
			break;
		case MID_POWERDOWN:
			printf("revc MID_POWERDOWN\n");
			g_verify_timeout = 0;
			g_enroll_timeout = 0;
			g_demo_mode = SIGN_DISABLE;
			
			/* 回复主控可断电 */
			ret = st_mod_to_host_mid(MID_REPLY, MID_POWERDOWN, MR_SUCCESS);
			if (ret != 0)
			{
				MY_ERR_PRT("host_msg_get", ret);
				return MY_ERRROR;
			}
			break;

		case MID_DEMOMODE:
		{
			printf("recv MID_DEMOMODE\n");

			s_msg_demomode_data *pst_demomode_data = (s_msg_demomode_data*)pst_mod_recv_msg->data;

			if (pst_demomode_data->enable)
				g_device_static = MS_BUSY;
			else
				g_device_static = MS_STANDBY;

			g_demo_mode = pst_demomode_data->enable;
			printf("g_demo_mode:%d\n", g_demo_mode);
			if (st_mod_to_host_mid(MID_REPLY, MID_DEMOMODE, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;
		}
		case MID_GET_INIT_TIME:
			printf("recv MID_GET_INIT_TIME\n");
			if (st_mod_to_host_mid(MID_REPLY, MID_GET_INIT_TIME, MR_SUCCESS) != 0)
				MY_ERR_PRT("st_mod_to_host_mid", MY_ERRROR);
			break;

		default:
			MY_ERR_PRT("st_mod_to_host_mid not support id", pst_mod_recv_msg->mid);
			ret = -5;
			break;
	}

	return ret;
}

int g_ready_tag = 0;

static int st_mod_recv_host_msg()
{
	int  ret;
	int timeout = 10; //1=10ms
	unsigned int msg_len = 0;
	s_msg st_recv_msg;
	s_msg *pst_mod_recv_msg = NULL;
	
	unsigned char *p_face_power_sign = face_power_sign();
	while (1)
	{
		if (*p_face_power_sign == SIGN_DISABLE){
			
			//printf("st_mod_recv_host_msg =*p_face_power_sign= %d and g_ready_tag= %d\n",*p_face_power_sign,g_ready_tag);
        		return -1;
        	}
		
        	
        	
        	//zcy add send ready for test
        	//printf("st_mod_recv_host_msg == %d and g_ready_tag= %d\n",ret,g_ready_tag);
        	#if READY_LOOP
        	if(g_ready_tag != 1000){
			if(g_ready_tag >= 5){
				g_ready_tag =0;
				st_mod_to_host_mid(MID_NOTE, NID_READY, MR_SUCCESS);//zcy add 
			}
			g_ready_tag++;
		}
		#endif
		/* 接收同步字高字 */
		ret = host_msg_get((char *)&st_recv_msg.sync_word_heb, timeout, sizeof(st_recv_msg.sync_word_heb));
		if (ret != 0)
		{
			//MY_ERR_PRT("host_msg_get", ret);
			continue;
		}

		if (st_recv_msg.sync_word_heb != SYNC_WORD_H)
		{
			MY_ERR_PRT("sync_word_h error", st_recv_msg.sync_word_heb);
			continue;
		}

		/* 接收同步字低字 */
		ret = host_msg_get((char *)&st_recv_msg.sync_word_leb, timeout, sizeof(st_recv_msg.sync_word_leb));
		if (ret != 0)
		{
			//MY_ERR_PRT("host_msg_get", ret);
			continue;
		}

		if (st_recv_msg.sync_word_leb != SYNC_WORD_L)
		{
			MY_ERR_PRT("sync_word_l error", st_recv_msg.sync_word_leb);
			continue;
			//break; //zcy add for test
		}
		else
			break;
			
		usleep(10*1000);	
	}
	
	
	
	
	if (g_enc_mode == 0x2)
	{
		s_enc_msg st_recv_enc_msg;
		/* 接收加密数据大小 */
		ret = host_msg_get((char*)&st_recv_enc_msg.size_heb, 100, 2);
		if (ret != 0) 
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}

		uint16_t msg_enc_data_len = (uint16_t)(st_recv_enc_msg.size_leb & 0xff) | (uint16_t)(st_recv_enc_msg.size_heb<<8);
		uint16_t msg_enc_len = msg_enc_data_len + sizeof(s_enc_msg);
		s_enc_msg *pst_recv_enc_msg = (s_enc_msg*)malloc(msg_enc_len);
		memset(pst_recv_enc_msg,0,msg_enc_len);//zcy add

		/* 接收加密数据和校验码 */
		host_msg_get((char*)pst_recv_enc_msg->data, 100, msg_enc_data_len + 1);
		if (ret != 0)
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}
		
		

		/* 加密parity_check */
		int i;
		uint8_t parity_check = 0;
		for (i=4; i<msg_enc_len - 1; i++)
		{
			parity_check^=((uint8_t*)pst_recv_enc_msg)[i];
			
		}

		if (parity_check != ((uint8_t*)pst_recv_enc_msg)[msg_enc_len-1])
		{
			printf("parity_check:%x msg_enc_data_len:%d pst_mod_recv_msg)[msg_len-1]:%x\n", parity_check, msg_enc_data_len, ((uint8_t*)pst_recv_enc_msg)[msg_enc_len-1]);
			MY_ERR_PRT("parity_check", parity_check);
			free(pst_recv_enc_msg);
			pst_recv_enc_msg = NULL;
			return MY_ERRROR;
		}

		msg_len =  msg_enc_data_len + 3;
		pst_mod_recv_msg = (s_msg*)malloc(msg_len);
		pst_mod_recv_msg->sync_word_heb = st_recv_msg.sync_word_heb;
		pst_mod_recv_msg->sync_word_heb = st_recv_msg.sync_word_leb;
		memcpy(&pst_mod_recv_msg->mid, pst_recv_enc_msg->data, msg_enc_data_len);

		decrypt_bytes(&pst_mod_recv_msg->mid, msg_enc_data_len, g_enc_key, 16, &pst_mod_recv_msg->mid);
		
		#if 0
		   printf("\n dec rx: \n ");
		   for(int k =0 ; k < msg_len; k++){
		   	printf("%02x ", ((uint8_t*)pst_mod_recv_msg)[k]);
		   }
		   printf("\n dec rx end : \n ");
		#endif
		

		free(pst_recv_enc_msg);
		pst_recv_enc_msg = NULL;
	}
	else if( g_enc_mode > 0x0){
		s_enc_msg st_recv_enc_msg;
		/* 接收加密数据大小 */
		ret = host_msg_get((char*)&st_recv_enc_msg.size_heb, 100, 2);
		if (ret != 0) 
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}

		uint16_t msg_enc_data_len = (uint16_t)(st_recv_enc_msg.size_leb & 0xff) | (uint16_t)(st_recv_enc_msg.size_heb<<8);
		uint16_t msg_enc_len = msg_enc_data_len + sizeof(s_enc_msg);
		s_enc_msg *pst_recv_enc_msg = (s_enc_msg*)malloc(msg_enc_len);
		memset(pst_recv_enc_msg,0,msg_enc_len);//zcy add

		/* 接收加密数据和校验码 */
		host_msg_get((char*)pst_recv_enc_msg->data, 100, msg_enc_data_len + 1);
		if (ret != 0)
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}
		
		/* 加密parity_check */
		int i;
		uint8_t parity_check = 0;
		for (i=4; i<msg_enc_len - 1; i++)
		{
			parity_check^=((uint8_t*)pst_recv_enc_msg)[i];
			
		}

		if (parity_check != ((uint8_t*)pst_recv_enc_msg)[msg_enc_len-1])
		{
			printf("parity_check:%x msg_enc_data_len:%d pst_mod_recv_msg)[msg_len-1]:%x\n", parity_check, msg_enc_data_len, ((uint8_t*)pst_recv_enc_msg)[msg_enc_len-1]);
			MY_ERR_PRT("parity_check", parity_check);
			free(pst_recv_enc_msg);
			pst_recv_enc_msg = NULL;
			return MY_ERRROR;
		}

		msg_len =  msg_enc_data_len + 3;
		pst_mod_recv_msg = (s_msg*)malloc(msg_len);
		pst_mod_recv_msg->sync_word_heb = st_recv_msg.sync_word_heb;
		pst_mod_recv_msg->sync_word_heb = st_recv_msg.sync_word_leb;
		memcpy(&pst_mod_recv_msg->mid, pst_recv_enc_msg->data, msg_enc_data_len);
		
	//	printf("msg_enc_data_len = %02x\n", msg_enc_data_len);
		
		if(msg_enc_data_len % 16 !=0)
		{
			MY_ERR_PRT("msg_enc_data_len aes len error", msg_enc_data_len);
			free(pst_mod_recv_msg);
			pst_mod_recv_msg = NULL;
			return MY_ERRROR;
		}
		
		#if UART_PRT
		   printf("\n before aes dec rx: \n ");
		   for(int k =0 ; k < msg_len; k++){
		   	printf("%02x ", ((uint8_t*)pst_mod_recv_msg)[k]);
		   }
		   printf("\n before aes dec rx end : \n ");
		#endif

	
		aesDecrypt(g_enc_key, 16, &pst_mod_recv_msg->mid, &pst_mod_recv_msg->mid, msg_enc_data_len);
		
		#if UART_PRT
		   printf("\n aes dec rx: \n ");
		   for(int k =0 ; k < msg_len; k++){
		   	printf("%02x ", ((uint8_t*)pst_mod_recv_msg)[k]);
		   }
		   printf("\n aes dec rx end : \n ");
		#endif
		
		
		free(pst_recv_enc_msg);
		pst_recv_enc_msg = NULL;
	}
	else
	{
		msg_len = sizeof(s_msg);
		/* 接收mid和数据大小 */
		ret = host_msg_get((char*)&st_recv_msg.mid, timeout, 3);
		if (ret != 0) 
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}

		uint16_t msg_data_len = (uint16_t)(st_recv_msg.size_leb & 0xff) | (uint16_t)(st_recv_msg.size_heb<<8);
		msg_len = sizeof(s_msg) + msg_data_len;
		pst_mod_recv_msg = (s_msg*)malloc(msg_len);
		memset(pst_mod_recv_msg,0,msg_len);//zcy add
		memcpy(pst_mod_recv_msg, &st_recv_msg, sizeof(s_msg) - 1);
		
		#if 0
		  
		   printf("no enc rx: msg len %d\n ",msg_len);
		   for(int k =0 ; k < msg_len; k++){
		   	printf("%02x ", ((uint8_t*)pst_mod_recv_msg)[k]);
		   }
		   printf("\n rx end : \n ");
	       #endif
		
		/* 接收消息内数据 */
		host_msg_get((char*)pst_mod_recv_msg->data, 100, msg_data_len + 1);
		if (ret != 0)
		{
			MY_ERR_PRT("host_msg_get", ret);
			return MY_ERRROR;
		}
		printf("msg_data_lenlen %d\n ",msg_data_len);
		/* parity_check */
		int i;
		uint8_t parity_check = 0;
		for (i=2; i<msg_len-1; i++)
		{
			parity_check^=((uint8_t*)pst_mod_recv_msg)[i];
			//printf("%2x ", ((uint8_t*)pst_mod_recv_msg)[i]);
		}
		if (parity_check != ((uint8_t*)pst_mod_recv_msg)[msg_len-1])
		{
			printf("parity_check:%x msg_len:%d pst_mod_recv_msg)[msg_len-1]:%x\n", parity_check, msg_len, ((uint8_t*)pst_mod_recv_msg)[msg_len-1]);	
			MY_ERR_PRT("parity_check", parity_check);
			free(pst_mod_recv_msg);
			pst_mod_recv_msg = NULL;
			return MY_ERRROR;
		}
		
	}

#if 0
	/* 检查是否有连续指令 */
	unsigned int msg_extra_sign = 0;
	s_msg st_recv_msg_extra;
	s_msg *pst_mod_recv_msg_extra = NULL;

	ret = host_msg_get((char *)&st_recv_msg_extra.sync_word_heb, 1, 2 * sizeof(st_recv_msg_extra.sync_word_heb));//10ms
	if (ret != 0)
	{
		MY_STATUS_PRT("not extra msg");
	}
	else if (st_recv_msg_extra.sync_word_heb == SYNC_WORD_H || st_recv_msg_extra.sync_word_leb == SYNC_WORD_L)
	{
		if (g_enc_mode == 0x2)
		{
			s_enc_msg st_recv_enc_msg_extra;
			/* 接收加密数据大小 */
			ret = host_msg_get((char*)&st_recv_enc_msg_extra.size_heb, 100, 2);
			if (ret != 0) 
			{
				MY_ERR_PRT("host_msg_get", ret);
				return MY_ERRROR;
			}

			uint16_t msg_enc_data_len = (uint16_t)(st_recv_enc_msg_extra.size_leb & 0xff) | (uint16_t)(st_recv_enc_msg_extra.size_heb<<8);
			uint16_t msg_enc_len = msg_enc_data_len + sizeof(s_enc_msg);
			s_enc_msg *pst_recv_enc_msg_extra = (s_enc_msg*)malloc(msg_enc_len);

			/* 接收加密数据和校验码 */
			host_msg_get((char*)pst_recv_enc_msg_extra->data, 100, msg_enc_data_len + 1);
			if (ret != 0)
			{
				MY_ERR_PRT("host_msg_get", ret);
				return MY_ERRROR;
			}

			/* 加密parity_check */
			int i;
			uint8_t parity_check = 0;
			for (i=4; i<msg_enc_len - 1; i++)
			{
				parity_check^=((uint8_t*)pst_recv_enc_msg_extra)[i];
				//printf("%2x ", ((uint8_t*)pst_mod_recv_msg)[i]);
			}

			if (parity_check != ((uint8_t*)pst_recv_enc_msg_extra)[msg_enc_len-1])
			{
				printf("parity_check:%x msg_enc_data_len:%d pst_recv_enc_msg_extra[msg_len-1]:%x\n", parity_check, msg_enc_data_len, ((uint8_t*)pst_recv_enc_msg_extra)[msg_enc_len-1]);
				MY_ERR_PRT("parity_check", parity_check);
				free(pst_recv_enc_msg_extra);
				pst_recv_enc_msg_extra = NULL;
				return MY_ERRROR;
			}

			msg_len =  msg_enc_data_len + 3;
			pst_mod_recv_msg_extra = (s_msg*)malloc(msg_len);
			pst_mod_recv_msg_extra->sync_word_heb = st_recv_msg_extra.sync_word_heb;
			pst_mod_recv_msg_extra->sync_word_heb = st_recv_msg_extra.sync_word_leb;
			memcpy(&pst_mod_recv_msg_extra->mid, pst_recv_enc_msg_extra->data, msg_enc_data_len);

			decrypt_bytes(&pst_mod_recv_msg_extra->mid, msg_enc_data_len, g_enc_key, 16, &pst_mod_recv_msg_extra->mid);

			free(pst_recv_enc_msg_extra);
			pst_recv_enc_msg_extra = NULL;

			if (pst_mod_recv_msg_extra->mid != pst_mod_recv_msg->mid)
			{
				msg_extra_sign = 1;
				MY_STATUS_PRT("same msg");
			}
		}
		else
		{
			msg_len = sizeof(s_msg);
			/* 接收mid和数据大小 */
			ret = host_msg_get((char*)&st_recv_msg_extra.mid, timeout, 3);
			if (ret != 0) 
			{
				MY_ERR_PRT("host_msg_get", ret);
				return MY_ERRROR;
			}

			uint16_t msg_data_len = (uint16_t)(st_recv_msg_extra.size_leb & 0xff) | (uint16_t)(st_recv_msg_extra.size_heb<<8);
			msg_len = sizeof(s_msg) + msg_data_len;
			pst_mod_recv_msg_extra = (s_msg*)malloc(msg_len);
			memcpy(pst_mod_recv_msg_extra, &st_recv_msg_extra, sizeof(s_msg) - 1);

			/* 接收消息内数据 */
			host_msg_get((char*)pst_mod_recv_msg_extra->data, 100, msg_data_len + 1);
			if (ret != 0)
			{
				MY_ERR_PRT("host_msg_get", ret);
				return MY_ERRROR;
			}

			/* parity_check */
			int i;
			uint8_t parity_check = 0;
			for (i=2; i<msg_len-1; i++)
			{
				parity_check^=((uint8_t*)pst_mod_recv_msg_extra)[i];
			}
			if (parity_check != ((uint8_t*)pst_mod_recv_msg_extra)[msg_len-1])
			{
				printf("parity_check:%x msg_len:%d pst_mod_recv_msg_extra[msg_len-1]:%x\n", parity_check, msg_len, ((uint8_t*)pst_mod_recv_msg_extra)[msg_len-1]);	
				MY_ERR_PRT("parity_check", parity_check);
				free(pst_mod_recv_msg_extra);
				pst_mod_recv_msg_extra = NULL;
				return MY_ERRROR;
			}

			if (pst_mod_recv_msg_extra->mid != pst_mod_recv_msg->mid)
			{
				msg_extra_sign = 1;
				MY_STATUS_PRT("same msg");
			}
		}
	}	
#endif

		#if UART_PRT
		  
		   printf("no enc rx 11: \n ");
		   for(int k =0 ; k < msg_len; k++){
		   	printf("%02x ", ((uint8_t*)pst_mod_recv_msg)[k]);
		   }
		   printf("\n rx end : \n ");
	       #endif
	ret = st_host_to_mod_mid(pst_mod_recv_msg);
	//zcy add for first right cmd set stop ready
	#if READY_LOOP
	if(ret == 0)
		g_ready_tag=1000;
	#endif
	
	//printf("st_host_to_mod_mid == %d\n",ret);
	if (ret != 0) {
		MY_ERR_PRT("public_host_to_mod_mid", ret);
		return MY_ERRROR;
	}

#if 0
	if (msg_extra_sign)
	{
		MY_STATUS_PRT("extra msg");
		ret = st_host_to_mod_mid(pst_mod_recv_msg_extra);
		if (ret != 0) {
			MY_ERR_PRT("public_host_to_mod_mid", ret);
			return MY_ERRROR;
		}
	}

	free(pst_mod_recv_msg_extra);
	pst_mod_recv_msg_extra = NULL;
#endif

	free(pst_mod_recv_msg);
	pst_mod_recv_msg = NULL;

	return 0;
}



/* 待机入口 */
int  standby_import()
{
	unsigned char *p_face_power_sign = face_power_sign();

	/* 通知主控已进入待机模式            MSG::NOTE::READY */
	int ret = st_mod_to_host_mid(MID_NOTE, NID_READY, MR_SUCCESS);
	if (ret != 0) {
		MY_ERR_PRT("public_mod_to_host_mid", ret);
		return MY_ERRROR;
	}

	clk_get_pts(&g_ready_pts);

	while (!init_sign_get())
		usleep(10*1000);

	/* 待机接收消息 */
	while (1)
	{
		//printf("standby_import  *p_face_power_sign == %d and SIGN_DISABLE= %d\n",*p_face_power_sign,SIGN_DISABLE);
		if (*p_face_power_sign == SIGN_DISABLE)
		{
			g_enc_mode =0 ; //zcy add
			/* 关闭tx复用     拉低引脚 */
			writel(0x0, 0x10020020);

			while (*p_face_power_sign == SIGN_DISABLE)
				usleep(10*1000);

			/* 开启tx复用     */
			writel(0x84, 0x10020020);

			usleep(200*1000);//zcy md 10 to 200
			//printf("standby_import *p_face_power_sign== %d and g_ready_tag= %d\n",*p_face_power_sign,g_ready_tag);

			/* 通知主控已进入待机模式            MSG::NOTE::READY */
			int ret = st_mod_to_host_mid(MID_NOTE, NID_READY, MR_SUCCESS);
			if (ret != 0) {
				MY_ERR_PRT("public_mod_to_host_mid", ret);
				return MY_ERRROR;
			}
		}

		int ret = st_mod_recv_host_msg();
		//printf("standby_import == %d and g_ready_tag= %d\n",ret,g_ready_tag);
		if (ret != 0 ){
			MY_ERR_PRT("public_mod_recv_host", ret);
			
		}
		

		usleep(10*1000);
	}

	return 0;
}

int uart_connect()
{
	if (uart_init(SAMPLE_UART_NAME, 115200) < 0)
		MY_ERR_PRT("uart_init", MY_ERRROR);
	
	if(standby_import() < 0) {
		MY_ERR_PRT("standby_import", MY_ERRROR);
		return -1;
	}

	return 0;
}




