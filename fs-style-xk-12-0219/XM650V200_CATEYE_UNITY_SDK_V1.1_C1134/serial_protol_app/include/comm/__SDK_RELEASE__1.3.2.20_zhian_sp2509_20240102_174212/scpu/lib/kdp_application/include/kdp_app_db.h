/*
 * Kneron Application Database APIs
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

/**
 * @file kdp_app_db.h
 * @brief Kneron Application DB APIs
 */

#ifndef __KAPP_DB_H_
#define __KAPP_DB_H_

#include <stdint.h>
#include "kdp_app.h"
#include "common.h"
#include "types.h"
#include "board_cfg.h"
#include "kdp_flash_table.h"

#if (CFG_FLASH_DB_NIR_ONLY != 1)
#define FR_TO_DB_IDX_0      0
#define FR_TO_DB_IDX_1      1
#define DB_NUM              2  //@brief DB count. 1 or 2
#else
#define FR_TO_DB_IDX_0      1 //rgb sensor
#define FR_TO_DB_IDX_1      0 //nir sendor
#define DB_NUM              1 //nir only
#endif

#define FM_SIZE             256
#define FM_SIZE_BYTE        (FM_SIZE * 4)
#define KDP_DB_DEFAULT_OFFSET  (0x01) //zcy mod 0x80 to 0x01

#define PALM_FM_SIZE        512

/** @brief flash address of DB */
#define KAPP_DB_FLASH_ADDR   KDP_FLASH_FID_MAP_ADDR
#define KAPP_PALM_DB_FLASH_ADDR    (KDP_FLASH_FID_MAP_ADDR + 16*1024*1024) //yg todo:

enum kdp_app_db_type_valid_flag_e{
    TYPE_INVALID = 0,
    TYPE_VALID,
    TYPE_REGISTER,
    TYPE_RESERVER,
};

enum kapp_db_status_code_e{
    KAPP_DB_NO_SPACE = 0x200,   //kdpApp.h
    KAPP_DB_ALREADY_SAVED,
    KAPP_DB_DEL_NOT_VALID,
    KAPP_DB_NO_MATCH,
    KAPP_DB_REG_FIRST,
    KAPP_DB_USER_NOT_REG,
    KAPP_DB_DEL_FAIL,
    KAPP_DB_FAIL_FLIP0,
    KAPP_DB_FAIL_FLIP1,
    KAPP_DB_FAIL_ABORT,
    KAPP_DB_POSE_ERR,
};

typedef struct kapp_db_user_data_s {
    uint16_t user_id_in;
    uint16_t user_id_out;
    uint16_t user_idx;
    uint16_t fm_idx;
    uint16_t del_all;
    uint16_t flash_off;
    uint16_t reserve_db_num;
} kapp_db_user_data_t;

typedef struct kdp_app_db_extra_data_struct {
    char user_name[MAX_LEN_USERNAME];
    u8 pre_add;            //Fixed 1bytes
    u8 admin;
    u8  rgb_age_group;
    u8 rgb_face_quality;
    u8 nir_face_quality;
//    u8 rgb_corner_y;
//    u16 rec_cnt;
//    u16 rec_fm_hist[MAX_FID];
    u8 nir_mode;
//    float fs_feature[17];
//    float darkness;
//    float effect_2d;
    float nir_gain;
    u32 nir_cur_exp_time;
    u16  n1_fd_x;
    u16  n1_fd_y;
    u16  n1_fd_w;
    u16  n1_fd_h;
    float lm_length_ratio[5];
} kdp_app_db_extra_data;

extern float g_nFaceL2;

int32_t kdp_app_db_extra_data_read(u8 user_idx, kdp_app_db_extra_data* data_addr);
int32_t kdp_app_db_extra_data_write(u8 user_idx, kdp_app_db_extra_data* data_addr);
int32_t kdp_reload_db(void);

/**
 * @brief register user fm data
 * @param [in] fdr_addr address of fd/fr feature map data
 * @param [in] user_id use id
 * @param [in] fm_idx face index
 * @param [in] reserve_db_num reserve more user_db uint in ddr memory
 * @return status
 */
//int32_t kdp_app_db_register(uint32_t fdr_addr, uint16_t user_id, uint16_t fm_idx, int reserve_db_num);

/**
 * @brief register user both fm data
 * @param [in] rgb_fdr_addr address of rgb fd/fr feature map data
 * @param [in] nir_fdr_addr address of nir fd/fr feature map data
 * @param [in] user_id use id
 * @param [in] fm_idx face index
 * @param [in] reserve_db_num reserve more user_db uint in ddr memory
 * @return status
 */
int32_t kdp_app_db_both_register(uint32_t rgb_fdr_addr, uint32_t nir_fdr_addr,  uint16_t user_id, uint16_t fm_idx, int reserve_db_num );

/**
 * @brief Initialize database
 * @param [in] flash_db_addr_p flash address of the database
 * @param [in] reserve_db_num reserve more user_db uint in ddr memory
 * @return registered user count
 */
//int32_t kdp_app_db_both_update( uint32_t rgb_fdr_addr, uint32_t nir_fdr_addr, uint16_t user_id, uint16_t fm_idx, int reserve_db_num, float coef);
int32_t kdp_app_db_fmap_update( uint32_t fdr_addr, uint16_t u_idx, uint16_t fm_idx, int db_idx);
void    kdp_app_db_save_fmap_to_flash(uint16_t update_idx, uint16_t fm_idx, int db_idx);

int32_t kdp_app_db_init(uint16_t reserve_db_num, BOOL flash_load );
int32_t kdp_app_db_init_flash_load(void);

/**
 * @brief switch between two DB if use two DB setting
 * @param [in] db_index indicate which DB is activated
 */
//void    kdp_app_db_switch(uint32_t db_index);

/**
 * @brief use two DB to store 2 kinds of FM data
 */
//void    kdp_app_db_split_mode(void);

/**
 * @brief register user data to database
 * @param [in] *p_inout_p p_inout_p->user_id user id
 * @return user index in database
 */
int32_t kdp_app_db_add(kapp_db_user_data_t *p_inout_p);

/**
 * @brief delete user data in ddr and flash
 * @param [in] *p_inout_p p_inout_p->user_id user id, "0" means all
 * @return none
 */
int32_t kdp_app_db_delete(kapp_db_user_data_t *p_inout_p);

/**
 * @brief output user number and all user id data
 */
//int32_t kdp_app_db_list(void);

/**
 * @brief output one feature map data of one user
 * @param [in] *p_inout_p p_inout_p->user_id
 *                        p_inout_p->image_idx
 */
//int32_t kdp_app_db_upload(kapp_db_user_data_t *p_inout_p);

/**
 * @brief clear all user data not yet valid
 * @return OK or error code
 */
int32_t kdp_app_db_abort_reg(void);

/**
 * @brief check if a db slot is used
 * @param [in] i slot index
 * @return 1: yes  0:no
 */
int32_t kdp_app_db_slot_is_used(uint16_t i);

/**
 * @brief get feature map 0 count for a user index
 * @param [in] user_idx user index
 * @return count for feature map 0
 */
//int kdp_app_db_get_fm0_cnt(int user_idx);

/**
 * @brief get feature map 1 count for a user index
 * @param [in] user_idx user index
 * @return count for feature map 1
 */
//int kdp_app_db_get_fm1_cnt(int user_idx);

/**
 * @brief get slot type for a user index
 * @param [in] user_idx user index
 * @return type, 0:invalid, 1:valid, 2:register
 */
//int kdp_app_db_get_user_id_slot_type(int user_idx);

/**
 * @brief get user ID for a user index
 * @param [in] user_idx user index
 * @return user ID (could be different from user index)
 */
int kdp_app_db_get_user_id(int user_idx);
u8  kdp_app_db_get_db_idx(u8 user_id);
u8  _kl520_app_calc_db_uid(u8 db_idx);

/**
 * @brief Add a single FM to DB,  init the user slot if slot is not valid
 * @param [in] user indx (slot #)
 * @param [in] user id
 * @param [in] u8 pointer to FM data (only works for fm0)
 * @return 0 - success, else error
 */
//int kdp_app_db_add_FM(int user_idx, int user_id, unsigned char *pfm_data);

/**
 * @brief Delete a single FM from DB, delete the user slot if no more FM is left
 * @param [in] user indx (slot #)
 * @param [in] FM indx (only works for fm0)
 * @return 0 - success, else error
 */
 //int kdp_app_db_delete_FM(int user_idx, int fm_idx);

/**
 * @brief Calculate similarity of two feature points
 * @param [out] **db_ddr_p output ddr address of DB temp
 * @param [out] **db_flash_p flash address of DB
 * @return similarity score (smaller means more like)
 */
int32_t kdp_app_db_query(uint32_t **db_ddr_p, uint32_t **db_flash_p);

/**
 * @brief compare user fd/fr result to user data in ddr (only normal face)
 * @param fdr_addr address of fd/fr feature map data
 * @param [in] user id
 * @param [in] database idx
 * @param thresh_fid threshold value for similarity
 * @return compare result and user id where find match feature map data
 */
//int32_t kdp_app_db_compare_normal(uint32_t fdr_addr, uint16_t *user_id, int db_idx, float thresh_fid);

/**
 * @brief compare user fd/fr result to itself
 * @param fdr_addr address of fd/fr feature map data
 * @param [in] user id
 * @param [in] database idx
 * @param [in] database idx
 * @param thresh_fid threshold value for comparing self
 * @param [in] reserve_db_num reserve more user_db uint in ddr memory
 * @return compare result
 */
int32_t kdp_app_db_compare_self(uint32_t fdr_addr, uint16_t user_id, int db_idx, float thresh_fid, int reserve_db_num);

/**
 * @brief compare user fd/fr result to user data in ddr
 * @param fdr_addr_1 address of feature map data
 * @param weight_1 fdr_addr_1 accounts for all the calculation weights
 * @param db_idx_1 index of databases related to fdr_addr_1
 * @param fdr_addr_2 address of feature map data
 * @param weight_2 fdr_addr_2 accounts for all the calculation weights
 * @param db_idx_2 index of databases related to fdr_addr_2
 * @param thresh_fid threshold value for comparison
 * @return compare result, min dist and user id where find match feature map data
 */
#if 0
int32_t kdp_app_db_compare_five_faces_both(
        uint32_t fdr_addr_1, float weight_1, int db_idx_1,
        uint32_t fdr_addr_2, float weight_2, int db_idx_2,
        float thresh_fid, uint16_t *user_id, float* min_dist/*output*/);
#endif

int32_t kdp_app_db_compare_five_faces_both_test(
        uint32_t fdr_addr_1, float weight_1, int db_idx_1,
        uint32_t fdr_addr_2, float weight_2, int db_idx_2,
        u16   nir_mode,    
        float thresh_fid, uint16_t *user_id/*output*/, float *min_dist, u8 var_pre_add);
int32_t kdp_app_db_compare_five_faces_one_user(
        uint32_t fdr_addr_1, float weight_1, int db_idx_1,
        uint32_t fdr_addr_2, float weight_2, int db_idx_2,
        u16  user_id, float thresh_fid);

uint16_t kdp_app_db_find_exist_id(uint16_t user_id);
void kdp_app_db_flash_to_ddr(u8 user_id);

int32_t kdp_app_db_get_user_info_by_idx(
        uint16_t idx,
        uint16_t *lp_valid_fm0,
        uint16_t *lp_valid_fm1,
        uint16_t *lp_type);

uint32_t kdp_app_db_get_user_addr( int user_id );
uint32_t kdp_app_db_get_user_size( void );
uint32_t kdp_app_db_get_user_setting_addr( u16 user_idx );

uint32_t kdp_app_db_get_all_info_user_addr( u16 idx );
uint32_t kdp_app_db_get_all_info_user_size(void);

void kdp_app_db_set_last_register_id_preprocess(uint16_t user_idx, uint8_t fm_active );
void kdp_app_db_set_last_register_id_postprocess( uint16_t user_idx );
int32_t kdp_app_db_2users_compare(uint8_t* pOutR1, uint32_t fdr_addr_r1_1, uint32_t fdr_addr_r1_2, float weight_r1, \
                                  uint8_t* pOutN1, uint32_t fdr_addr_n1_1, uint32_t fdr_addr_n1_2, float weight_n1 );

BOOL kdp_app_imp_db_chk(void);
BOOL kdp_app_imp_fm_chk(BOOL nAfterChk);
BOOL kdp_app_imp_fm_chk_int8(u32 addr);

//void kdp_app_all_db_move_to_flash( uint32_t nSize );
//void kdp_app_update_all_user_id(void);

//import DB data
//if specified slot is not empty, return error if force_mode is 0. 
//return the new user id and DB data
int32_t kdp_app_db_import(uint16_t user_id, uint8_t* data, uint16_t data_size);
int32_t kdp_app_db_import_request(uint16_t *p_user_id, uint32_t size, uint8_t mode);

bool kdp_app_db_check_user_id(uint16_t user_id);
bool kdp_app_db_check_usr_idx(u8 usr_idx);

#endif /* __KAPP_DB_H_ */
