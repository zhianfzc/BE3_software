#include "user_ui.h"

#include "board_ddr_table.h"
#include "board_flash_table.h"
#include "framework/event.h"
#include "kdp_memxfer.h"
#include "pinmux.h"
#include "kdp520_gpio.h"
#include "sample_gui_fsm_events.h"


#define LOAD_IMAGES_IN_USER_UI_INIT
#define MEMXFER_USE_DRIVER_STYLE

#ifndef MEMXFER_USE_DRIVER_STYLE
int kdp_memxfer_init(u8 flash_mode, u8 mem_mode);
int kdp_memxfer_flash_to_ddr(u32 dst, u32 src, size_t bytes);
#endif


void kl520_api_ddr_img_user(void)
{
    kdp_memxfer_flash_to_ddr(USR_DDR_SETTINGS_ADDR, USR_FLASH_SETTINGS_ADDR, USR_FLASH_SETTINGS_SIZE);

#if (CFG_PANEL_TYPE > PANEL_NULL)
#if (CFG_GUI_ENABLE == YES)
#if ( GUI_VERSION_TYPE == GUI_V1)
    //Main page
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_BTN_PW_E_ADDR, USR_FLASH_IMG_BTN_PW_E_ADDR, USR_FLASH_IMG_BTN_PW_E_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_BTN_FACE_E_ADDR, USR_FLASH_IMG_BTN_FACE_E_ADDR, USR_FLASH_IMG_BTN_FACE_E_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_BTN_GOON_E_ADDR, USR_FLASH_IMG_BTN_GOON_E_ADDR, USR_FLASH_IMG_BTN_GOON_E_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_DEL_ID_ADDR, USR_FLASH_IMG_ICON_DEL_ID_ADDR, USR_FLASH_IMG_ICON_DEL_ID_SIZE);

#elif ( GUI_VERSION_TYPE == GUI_V2)
    //Main page
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_FACE_DATA_ADDR, USR_FLASH_IMG_FACE_DATA_ADDR, USR_FLASH_IMG_FACE_DATA_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_FACE_ADDR, USR_FLASH_IMG_FACE_ADDR, USR_FLASH_IMG_FACE_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_PASSWORD_ADDR, USR_FLASH_IMG_PASSWORD_ADDR, USR_FLASH_IMG_PASSWORD_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_SETTING_ADDR, USR_FLASH_IMG_SETTING_ADDR, USR_FLASH_IMG_SETTING_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_POWER_ADDR, USR_FLASH_IMG_POWER_ADDR, USR_FLASH_IMG_POWER_SIZE);

    //Register management
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_F_REG_NEW_ADDR, USR_FLASH_IMG_F_REG_NEW_ADDR, USR_FLASH_IMG_F_REG_NEW_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_F_DEL_ALL_ADDR, USR_FLASH_IMG_F_DEL_ALL_ADDR, USR_FLASH_IMG_F_DEL_ALL_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_F_DEL_ONE_ID_ADDR, USR_FLASH_IMG_F_DEL_ONE_ID_ADDR, USR_FLASH_IMG_F_DEL_ONE_ID_SIZE);

    //Password management
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_PW_ADD_ADDR, USR_FLASH_IMG_PW_ADD_ADDR, USR_FLASH_IMG_PW_ADD_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_PW_DEL_ADDR, USR_FLASH_IMG_PW_DEL_ADDR, USR_FLASH_IMG_PW_DEL_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_PW_UNLOCK_ADDR, USR_FLASH_IMG_PW_UNLOCK_ADDR, USR_FLASH_IMG_PW_UNLOCK_SIZE);

    //System page
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_SYS_ICON_ADDR, USR_FLASH_IMG_SYS_ICON_ADDR, USR_FLASH_IMG_SYS_ICON_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_SYS_FW_INFO_ADDR, USR_FLASH_IMG_SYS_FW_INFO_ADDR, USR_FLASH_IMG_SYS_FW_INFO_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_SYS_UUID_ADDR, USR_FLASH_IMG_SYS_UUID_ADDR, USR_FLASH_IMG_SYS_UUID_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_SYS_NUM_ADDR, USR_FLASH_IMG_SYS_NUM_ADDR, USR_FLASH_IMG_SYS_NUM_SIZE);

    //Back and Home
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_RETURN_ADDR, USR_FLASH_IMG_RETURN_ADDR, USR_FLASH_IMG_RETURN_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_HOME_ADDR, USR_FLASH_IMG_HOME_ADDR, USR_FLASH_IMG_HOME_SIZE);

//    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_F_DEL_ONE_FACE_ADDR, USR_FLASH_IMG_F_DEL_ONE_FACE_ADDR, USR_FLASH_IMG_F_DEL_ONE_FACE_SIZE);
#endif

    //Keypad image
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_STAR_ADDR, USR_FLASH_IMG_DIGIT_STAR_ADDR, USR_FLASH_IMG_DIGIT_STAR_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_1_ADDR, USR_FLASH_IMG_DIGIT_1_ADDR, USR_FLASH_IMG_DIGIT_1_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_2_ADDR, USR_FLASH_IMG_DIGIT_2_ADDR, USR_FLASH_IMG_DIGIT_2_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_3_ADDR, USR_FLASH_IMG_DIGIT_3_ADDR, USR_FLASH_IMG_DIGIT_3_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_4_ADDR, USR_FLASH_IMG_DIGIT_4_ADDR, USR_FLASH_IMG_DIGIT_4_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_5_ADDR, USR_FLASH_IMG_DIGIT_5_ADDR, USR_FLASH_IMG_DIGIT_5_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_6_ADDR, USR_FLASH_IMG_DIGIT_6_ADDR, USR_FLASH_IMG_DIGIT_6_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_7_ADDR, USR_FLASH_IMG_DIGIT_7_ADDR, USR_FLASH_IMG_DIGIT_7_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_8_ADDR, USR_FLASH_IMG_DIGIT_8_ADDR, USR_FLASH_IMG_DIGIT_8_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_9_ADDR, USR_FLASH_IMG_DIGIT_9_ADDR, USR_FLASH_IMG_DIGIT_9_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DIGIT_0_ADDR, USR_FLASH_IMG_DIGIT_0_ADDR, USR_FLASH_IMG_DIGIT_0_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_SUCCESS_ADDR, USR_FLASH_IMG_ICON_SUCCESS_ADDR, USR_FLASH_IMG_ICON_SUCCESS_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_FAIL_ADDR, USR_FLASH_IMG_ICON_FAIL_ADDR, USR_FLASH_IMG_ICON_FAIL_SIZE);
#endif

    //Success and fail image
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_RECOGNIZE_SUCCESSED_ADDR, USR_FLASH_IMG_RECOGNIZE_SUCCESSED_ADDR, USR_FLASH_IMG_RECOGNIZE_SUCCESSED_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_RECOGNIZE_FAILED_ADDR, USR_FLASH_IMG_RECOGNIZE_FAILED_ADDR, USR_FLASH_IMG_RECOGNIZE_FAILED_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_REGISTER_SUCCESSED_ADDR, USR_FLASH_IMG_REGISTER_SUCCESSED_ADDR, USR_FLASH_IMG_REGISTER_SUCCESSED_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_REGISTER_FAILED_ADDR, USR_FLASH_IMG_REGISTER_FAILED_ADDR, USR_FLASH_IMG_REGISTER_FAILED_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DELETE_SUCCESSED_ADDR, USR_FLASH_IMG_DELETE_SUCCESSED_ADDR, USR_FLASH_IMG_DELETE_SUCCESSED_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_DELETE_FAILED_ADDR, USR_FLASH_IMG_DELETE_FAILED_ADDR, USR_FLASH_IMG_DELETE_FAILED_SIZE);

    //Direction
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_UP_ADDR, USR_FLASH_IMG_ICON_ARROW_UP_ADDR, USR_FLASH_IMG_ICON_ARROW_UP_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_DOWN_ADDR, USR_FLASH_IMG_ICON_ARROW_DOWN_ADDR, USR_FLASH_IMG_ICON_ARROW_DOWN_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_LEFT_ADDR, USR_FLASH_IMG_ICON_ARROW_LEFT_ADDR, USR_FLASH_IMG_ICON_ARROW_LEFT_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_RIGHT_ADDR, USR_FLASH_IMG_ICON_ARROW_RIGHT_ADDR, USR_FLASH_IMG_ICON_ARROW_RIGHT_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_UP_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_UP_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_UP_OK_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_DOWN_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_DOWN_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_DOWN_OK_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_LEFT_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_LEFT_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_LEFT_OK_SIZE);
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_ICON_ARROW_RIGHT_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_RIGHT_OK_ADDR, USR_FLASH_IMG_ICON_ARROW_RIGHT_OK_SIZE);

    //Text
    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_RECOGNIZE_TEXT_ADDR, USR_FLASH_IMG_RECOGNIZE_TEXT_ADDR, USR_FLASH_IMG_RECOGNIZE_TEXT_SIZE);

//    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_BTN_NEXT_E_ADDR, USR_FLASH_IMG_BTN_NEXT_E_ADDR, USR_FLASH_IMG_BTN_NEXT_E_SIZE);
//    kdp_memxfer_flash_to_ddr(USR_DDR_IMG_RESERVATION_ADDR, USR_FLASH_IMG_RESERVATION_ADDR, USR_FLASH_IMG_RESERVATION_SIZE);
#endif
}

void kl520_api_ddr_img_init(void)
{
    kdp_memxfer_init(MEMXFER_OPS_CPU, MEMXFER_OPS_CPU);

#if (CFG_SNAPSHOT_ADVANCED == 1)
    kdp_memxfer_flash_to_ddr(KDP_DDR_TEST_RGB_IMG_ADDR, KDP_FLASH_RGB_ADDR, KDP_FLASH_RGB_SIZE);
    kdp_memxfer_flash_to_ddr(KDP_DDR_TEST_NIR_IMG_ADDR, KDP_FLASH_NIR_ADDR, KDP_FLASH_NIR_SIZE);
#endif
}

void user_ui_init(void)
{
    kl520_api_ddr_img_init();
#ifdef LOAD_IMAGES_IN_USER_UI_INIT
    kl520_api_ddr_img_user();
#endif
}