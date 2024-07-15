/*******************************************************************************
Copyright © XmSilicon Tech Co., Ltd. 2022-2022. All rights reserved.
文件名: input.h
作者@编号： XM
功能描述: 录入相关头文件
******************************************************************************/
#ifndef __INPUT_H__
#define __INPUT_H__

typedef struct USR_INFO_S
{
	unsigned char  usr_sign;      	/* 用户特征 0：不存在 1：存在*/
	unsigned char  admin;
		  	/* 是否管理员 */
	unsigned char  ia_feat_lack;
  	/* 特征值是否缺乏 */
	unsigned char  user_name[32];
}USR_INFO_S;

typedef struct USR_LIST_INFO_S
{
    unsigned char  	usr_num;			/* 底库人数 */
    //unsigned char 	verify_threshold_level;	/* 算法识别等级 */
    //unsigned char 	liveness_threshold_level;	/* 算法活体等级 */
	USR_INFO_S 		sub_usr_list[100];	/* 子用户列表 */
} USR_LIST_INFO_S;

/* 用户加载类型类型     */
typedef enum
{
	ADD_END = 0,
	ADD_INPUTING = 1,
	ADD_INPUTING2 = 2,
	ADD_RELOAD = 3,
	ADD_JUDGE_DUP_FACE = 4,
	ADD_DUP_FACE = 5,
	ADD_NO_DUP_FACE = 6,
	ADD_BUTT
}add_type_e;

extern USR_LIST_INFO_S *usr_list_info_get();

unsigned char signle_enroll_usr_id_get();
unsigned char enroll_usr_id_get();
unsigned char curr_input_face_get();

int usr_empty();
int user_delete(unsigned int delete_usr_id);
int ia_notice_usr(unsigned int cmd);


int key_rule_store(uint8_t *p_enc_key_number);

void key_rule_get(uint8_t *p_enc_key_number);


#endif

