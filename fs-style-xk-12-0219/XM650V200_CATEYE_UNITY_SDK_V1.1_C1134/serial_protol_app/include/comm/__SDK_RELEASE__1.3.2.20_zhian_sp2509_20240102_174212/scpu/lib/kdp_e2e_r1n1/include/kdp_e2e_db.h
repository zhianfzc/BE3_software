#ifndef __KDP_E2E_DB_H__
#define __KDP_E2E_DB_H__


#include "types.h"
#include "kdp_app_db.h"
#include "kdp_e2e_face.h"

#define DB_FACE_ADD_IDLE    0
#define DB_FACE_ADD_GOING   1
#define DB_FLASH_WRITE      2
#define DB_FLASH_NO_WRITE   3

#if CFG_FACE_FR50M == 1
#define DB_DEFAULT_FR_THRESHOLD_LEVEL   2
#else
#define DB_DEFAULT_FR_THRESHOLD_LEVEL   4
#endif

//typedef struct kdp_e2e_db_extra_data_struct {
//    //u32 user_id;
//    char user_name[MAX_LEN_USERNAME];
//    u8 pre_add;            //Fixed 1bytes
//    u8 admin;
//    u8  rgb_age_group;
//    u8 rgb_face_quality;
//    u8 nir_face_quality;
//    u8 rgb_corner_y;
//    u16 rec_cnt;
//    u16 rec_fm_hist[MAX_FID];
//    kdp_nir_mode nir_mode;
//    float fs_feature[17];
//    float darkness;
//    float effect_2d;
//    float nir_gain;
//    u32 nir_cur_exp_time;
//    u16  n1_fd_x;
//    u16  n1_fd_y;
//    u16  n1_fd_w;
//    u16  n1_fd_h;
//    float lm_length_ratio[5];
//} kdp_e2e_db_extra_data;

typedef kdp_app_db_extra_data kdp_e2e_db_extra_data;

kapp_db_user_data_t* kdp_e2e_get_db_data(void);
void kdp_e2e_db_init( uint16_t reserve_db_num );
void kdp_e2e_db_init_flash_load(void);
void kdp_e2e_db_free(void);
//s32 kdp_e2e_db_compare_normal(u16 *lp_user_id);
s32 kdp_e2e_db_compare_self(u16 lp_user_id, u8 reg_idx, int reserve_db_num);
s32 kdp_e2e_db_compare(u16 *lp_user_id, float* thres_arr);
void kdp_e2e_db_init_thres( void );
//void kdp_e2e_db_get_thres( float *min_dis, float *db_thres );
void kdp_e2e_db_set_thres( float min_dis, float db_thres );
s32 kdp_e2e_db_compare_one_user(u16 user_id);
s32 kdp_e2e_db_register(u16 user_id, u16 fmap_idx, int reserve_db_num );
s32 kdp_e2e_db_recog_update(u16 user_id, u16 fmap_idx, int reserve_db_num, float coef );
s32 kdp_e2e_db_add(u16 user_id, BOOL db_wo_flash, u16 reserve_db_num );
//s32 kdp_e2e_db_update(u16 user_id, BOOL db_wo_flash, u16 reserve_db_num );
s32 kdp_e2e_db_delete(u8 del_ctrl, u16 user_id);
s32 kdp_e2e_db_get_user_info(u16 user_id, u16 *lp_valid_fm0, u16 *lp_valid_fm1, u16 *lp_type);
s32 kdp_e2e_db_get_user_info_by_idx(u16 idx, u16 *lp_valid_fm0, u16 *lp_valid_fm1, u16 *lp_type);
s32 kdp_e2e_db_abort_reg(void);

s32 kdp_e2e_db_extra_read(u8 user_idx, kdp_e2e_db_extra_data *data_addr, u32 data_size);
s32 kdp_e2e_db_2user_compare( uint8_t* pOutR1, uint32_t fdr_addr_r1_1,uint32_t fdr_addr_r1_2, \
                              uint8_t* pOutN1, uint32_t fdr_addr_n1_1,uint32_t fdr_addr_n1_2 );

s32 kdp_e2e_db_write_lock(void);
s32 kdp_e2e_db_write_unlock(void);
s32 kdp_e2e_db_get_total_num(void);

void kdp_e2e_set_fr_threshold_level(u8 level);
u8   kdp_e2e_get_fr_threshold_level(void);

void kdp_e2e_update_user_id(u16 user_idx, u16 user_id);

#endif
