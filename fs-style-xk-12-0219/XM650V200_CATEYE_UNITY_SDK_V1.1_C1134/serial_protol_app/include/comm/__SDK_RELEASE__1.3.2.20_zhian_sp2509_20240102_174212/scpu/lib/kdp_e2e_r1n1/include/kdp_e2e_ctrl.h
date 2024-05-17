#ifndef __KDP_E2E_CTRL_H__
#define __KDP_E2E_CTRL_H__


#include "types.h"
#include "ipc.h"
#include "board_kl520.h"
#include "kdp_e2e_n1_only.h"
#include "kdp_e2e_n1r1.h"
#include "kdp_e2e_face.h"
//#define VOTE  YES

#define FACE_DARK_TH        1
#define FACE_QUALITY_TH     2
#define ENV_BAD_TH          4
#define FACE_BACK_LIGHT_TH  2
#define TARGET_LUMA         80
#define OPEN_RGB_LED_2S
#define OPEN_RGB_LED_2S_CNT 5

extern u8 tile_avg_valid_x;
extern u8 tile_avg_valid_y;
#if ( CFG_PALM_PRINT_MODE == YES )
extern s16 brightness_stats_0[TILE_AVG_BLOCK_NUMBER];
extern s16 brightness_stats_1[TILE_AVG_BLOCK_NUMBER];
extern s16 brightness_stats_bg[TILE_AVG_BLOCK_NUMBER];
extern s16 brightness_stats_diff[TILE_AVG_BLOCK_NUMBER];
extern u8 hand_cnt;
#endif

struct kdp_e2e_ctrl_tile {
    u16 tile_mean;
    u8 tile_max;
    u8 tile_min;
    u8 tile_saturation_cnt;
    u8 tile_max_x;
    u8 tile_max_y;
};

#if ( CFG_PALM_PRINT_MODE == YES )
typedef struct
{
    u8  block_num_x;
    u8  block_num_y;
    u16 block_size_x;
    u16 block_size_y;
    u16 cal_start_x;   //pixel
    u16 cal_start_y;   //pixel
    u8  cal_step;      //pixel
} brightness_stats_info;
#endif

typedef void (*fn_led_open)(u16);
typedef void (*fn_led_close)(void);
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 ) || ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36404 )
typedef void (*fn_led_light_mode)(u16);
#endif

typedef struct kdp_e2e_ctrl_led_struct {
    fn_led_open         led_open;
    fn_led_close        led_close;
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 ) || ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36404 )
    fn_led_light_mode   led_light_mode;
#endif  
} kdp_e2e_ctrl_led;

typedef int (*fn_strength_get)(void);
typedef struct kdp_e2e_ctrl_light_sensor_struct {
    fn_strength_get strength_get;
} kdp_e2e_ctrl_light_sensor;
typedef struct kdp_e2e_ctrl_ops_struct {
    kdp_e2e_ctrl_led leds[IMGSRC_NUM];
    kdp_e2e_ctrl_light_sensor light_sensor;
} kdp_e2e_ctrl_ops;


kdp_e2e_ctrl_ops* kdp_e2e_ctrl_get_ops(void);
#if ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36515 ) || ( CFG_LED_DRIVER_TYPE == LED_DRIVER_AW36404 )
void kdp_e2e_ctrl_led_register(u32 led_src_idx, fn_led_open fn_open, fn_led_close fn_close, fn_led_light_mode fn_mode );
#else
void kdp_e2e_ctrl_led_register(u32 led_src_idx, fn_led_open fn_open, fn_led_close fn_close);
#endif
void kdp_e2e_ctrl_light_sensor_register(fn_strength_get fn_get);
BOOL kdp_e2e_ctrl_r1_led_tuning_when_normal(struct facedet_result_s *r1_fd, struct landmark_result_s *r1_lm);
BOOL kdp_e2e_ctrl_r1_led_tuning_when_error(void);
void kdp_e2e_rgb_led_open_register(void);
#if (CFG_AI_TYPE == AI_TYPE_N1R1)
s32 kdp_e2e_ctrl_rgb_led_n1r1(void);
//void kdp_e2e_n1r1_get_luma_ratio(void);
#endif
void kdp_e2e_nir_led_flag_on(void);
void kdp_e2e_nir_led_flag_off(void);
#if (E2E_N1_ONLY_STRUCTURE_ENABLE == YES)
void kdp_e2e_n1_off_str_on_switch(void);
s32 kdp_e2e_n1_on_str_off_switch(void);
#endif
#if (E2E_N1_TWO_STAGE_LED_MODE == YES)
s32 kdp_e2e_n1_switch_to_normal_led(void);
void kdp_e2e_n1_switch_to_flash_led(u8 level);
#endif

void kdp_e2e_n1_on_str_off_gain_exp_recorver(void);

#if VOTE == YES
int vote(u8 *ret_cnt, int *ret);
#endif

#if (AEC_MULTI_THREAD == YES)
void write_chk_n1(e2e_nir_cam_params* nir_cam_params, kdp_e2e_face_variables *vars, s32 ret);
void write_chk_r1(e2e_rgb_cam_params* rgb_cam_params, kdp_e2e_face_variables *vars, s32 ret);
#endif

int kdp_e2e_nir_tile_value_mean(u8 start_x, u8 end_x, u8 start_y, u8 end_y, struct kdp_e2e_ctrl_tile *tile_info);
s32 kdp_e2e_nir_led_open(void);
s32 kdp_e2e_ctrl_aec(void);
void kdp_e2e_rgb_led_gradually(void);

#if ( CFG_PALM_PRINT_MODE == YES )
void kdp_e2e_brightness_stats_show(u8 block_w, u8 block_h, s16 value[]);
u8 kdp_e2e_aec_brightness_palm_search( u8 block_w, u8 block_h ); 
void kdp_e2e_aec_brightness_stats_set_block( uint16_t size_x, uint16_t size_y, uint8_t step );
void kdp_e2e_aec_brightness_stats_set_area( uint8_t block_x, uint8_t block_y, uint16_t start_x, uint16_t start_y );
int kdp_e2e_aec_brightness_stats_cpu_calculate( uint8_t* img_data, uint16_t img_width, uint16_t img_hight, s16 *aec_stats_arr, struct kdp_e2e_ctrl_tile *tile_info );
#endif
#endif
