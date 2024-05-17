#include "board_kl520.h"
#include <stdlib.h>
#include <stdarg.h>
#include "drivers.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "board_ddr_table.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_fdfr.h"
#include "kdp_uart.h"
#include "pinmux.h"
#include "flash.h"
#include "kdp_memxfer.h"
#include "kdp_memory.h"
#include "kdp_e2e_settings.h"
#include "kdp_e2e_face.h"
#include "kl520_api_device_id.h"
#include "kl520_api_sim.h"
#include "kdp520_dma.h"
#include "media/display/display.h"
#include "media/display/video_renderer.h"
#include "kl520_api_camera.h"
#include "user_comm_msg_define.h"
#include "kdp_model.h"

#define SNAPSHOT_LOG_ENABLE

//global var
#if ( CFG_COM_BUS_TYPE & COM_BUS_UART_MASK )
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE
u8 g_snap_img_cnt = 0;
#endif
#endif

#if CFG_SNAPSHOT_ENABLE == 1
u8  snapshot_status[SNAPSHOT_SRC_NUM];
osMutexId_t snap_sts_mtx = NULL;
osEventFlagsId_t snapshot_evt = NULL;
#else
u32 face_add_addr[SNAPSHOT_SRC_NUM][SNAPSHOT_ADV_NUM]={0};
#endif

#if CFG_USB_SIMTOOL == 1 || CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
//local var
static u32 snapshot_addr[SNAPSHOT_SRC_NUM];

//local func
static u32 InfoBinGen(u32 addr, char *buf, const char *fmt, ...)
{
    //static u32 addr = 0;
    u32 len;
    //int n;
    va_list ap;
    va_start(ap, fmt);
    vsprintf (buf, fmt, ap);//n = vsprintf (buf, fmt, ap);
    va_end(ap);
    len = strlen(buf);

    //DDR Ckeck Head
    if(addr == 0)
    {
        memset((void *)(snapshot_addr[MIPI_CAM_INF]), 0, KDP_DDR_TEST_INF_IMG_SIZE);
        addr = snapshot_addr[MIPI_CAM_INF];
    }

    //DDR Ckeck End
    if( addr + len >= (snapshot_addr[MIPI_CAM_INF] + KDP_DDR_TEST_INF_IMG_SIZE) )
    {
        dbg_msg_console("info ddr over len");
        return 0;
    }

    memcpy((void *)(addr), (void *)buf, len);
    addr = addr + len;
    return (addr);
}

static void kl520_api_snapshot_get_inf(void)
{
    dbg_msg_algo ("write back JSON info file...");
    u8 i;
    char str[256];
    u32 offset = 0;
    struct nir_camera_tune_s *p_nir_tune_p = kdp_e2e_get_r1n1_tune();
    kdp_e2e_settings* settings = kdp_e2e_settings_get_inst();
    kdp_e2e_face_variables *face_var = kdp_e2e_get_face_variables();
    static system_info snap_sys_info = { 0 };

    if(snap_sys_info.fw_scpu_version.date == 0)
    {
        kl520_api_get_version_info(&snap_sys_info);
        kl520_api_free_device_info(&snap_sys_info);
    }

    offset = InfoBinGen(offset, str, "{\n");

    offset = InfoBinGen(offset, str, "\"System parameter(sys)\":\n");
    offset = InfoBinGen(offset, str, "{\n");
    offset = InfoBinGen(offset, str, "\"boot loader version\": \"0x%x\",\n",snap_sys_info.unique_id);
    offset = InfoBinGen(offset, str, "\"scpu firmware version\": \"%d.%d.%d.%d_%d\",\n", snap_sys_info.fw_scpu_version.version[0],\
                                                                                                snap_sys_info.fw_scpu_version.version[1],\
                                                                                                snap_sys_info.fw_scpu_version.version[2],\
                                                                                                snap_sys_info.fw_scpu_version.version[3],\
                                                                                                snap_sys_info.fw_scpu_version.date);
    offset = InfoBinGen(offset, str, "\"ncpu firmware version\": \"%d.%d.%d.%d_%d\",\n", snap_sys_info.fw_ncpu_version.version[0],\
                                                                                                snap_sys_info.fw_ncpu_version.version[1],\
                                                                                                snap_sys_info.fw_ncpu_version.version[2],\
                                                                                                snap_sys_info.fw_ncpu_version.version[3],\
                                                                                                snap_sys_info.fw_ncpu_version.date);
    offset = InfoBinGen(offset, str, "\"sim\": %d,\n",kl520_api_sim_is_running());
    offset = InfoBinGen(offset, str, "\"run\": %d,\n",kl520_api_sim_get_count());
    offset = InfoBinGen(offset, str, "\"cmd\": %d,\n",kl520_api_sim_get_usercmd());
    offset = InfoBinGen(offset, str, "\"fdfr\": %d,\n",kl520_api_sim_get_fdfr());
#ifdef CFG_CAMERA_ROTATE    
    offset = InfoBinGen(offset, str, "\"camera_rotate\": %d,\n",CFG_CAMERA_ROTATE);
#else
    offset = InfoBinGen(offset, str, "\"camera_rotate\": %d,\n",0);
#endif
    {
    for (int i = 0; i < snap_sys_info.model_count; ++i) {
    if(i == snap_sys_info.model_count-1)
    offset = InfoBinGen(offset, str, "\"model[%d] type0x%x version\": %d\n",i,snap_sys_info.model_infos[i]->model_type,snap_sys_info.model_infos[i]->model_version);
    else
    offset = InfoBinGen(offset, str, "\"model[%d] type0x%x version\": %d,\n",i,snap_sys_info.model_infos[i]->model_type,snap_sys_info.model_infos[i]->model_version);
    }
    }
    offset = InfoBinGen(offset, str, "},\n");

    offset = InfoBinGen(offset, str, "     \"Liveness parameter(alg) nir tuned\":\n");
    offset = InfoBinGen(offset, str, "     {\n");
    offset = InfoBinGen(offset, str, "           \"CFG_BOARD_PARAMS_0_ID\": %d,\n",CFG_BOARD_PARAMS_0_ID);                      //0
    offset = InfoBinGen(offset, str, "           \"init_tile_flag\": %d,\n",face_var->init_tile_flag);                          //1
    offset = InfoBinGen(offset, str, "           \"init_exp_tile\": %d,\n",face_var->init_exp_tile);                            //2
    offset = InfoBinGen(offset, str, "           \"nir_mode\": %d,\n",face_var->nir_mode);                                      //3
    offset = InfoBinGen(offset, str, "           \"init_nir_gain\": %f,\n",face_var->init_nir_gain);                            //4
    offset = InfoBinGen(offset, str, "           \"nir_gain\": %f,\n",face_var->nir_gain);                                      //5
    offset = InfoBinGen(offset, str, "           \"nir_cur_exp_time\": %d,\n",face_var->nir_cur_exp_time);                      //6

    offset = InfoBinGen(offset, str, "           \"rgb_led_flag\": %d,\n",face_var->rgb_led_flag);                              //7
    offset = InfoBinGen(offset, str, "           \"rgb_cur_exp_time\": %d,\n",face_var->rgb_cur_exp_time);                      //8
    offset = InfoBinGen(offset, str, "           \"lm_diff\": %d,\n",face_var->lm_diff);                                        //9
    offset = InfoBinGen(offset, str, "           \"rgb_nir_fr_diff\": %f,\n",face_var->rgb_nir_fr_diff);                        //10
    offset = InfoBinGen(offset, str, "           \"nir_lv_cnn_real_score\": %f,\n",face_var->nir_lv_cnn_real_score);            //11
    offset = InfoBinGen(offset, str, "           \"fuse_lv_real_score\": %f,\n",face_var->fuse_lv_real_score);                  //12
    offset = InfoBinGen(offset, str, "           \"registered_offsetX\": %f,\n",settings->registered_offsetX);                  //13
    offset = InfoBinGen(offset, str, "           \"registered_offsetY\": %f,\n",settings->registered_offsetY);                  //14
    offset = InfoBinGen(offset, str, "           \"nir_luma_ratio\": %f,\n",face_var->nir_luma_ratio);                          //15
    offset = InfoBinGen(offset, str, "           \"rgb_lv_cnn_real_score\": %f, \n",face_var->rgb_lv_cnn_real_score);           //16
    offset = InfoBinGen(offset, str, "           \"nir_lv_cnn_diff\": %f,\n",face_var->nir_lv_cnn_diff);                        //17
    offset = InfoBinGen(offset, str, "           \"fuse_lv_diff\": %f,\n",face_var->fuse_lv_diff);                              //18
    offset = InfoBinGen(offset, str, "           \"rgb_lv_cnn_diff\": %f,\n",face_var->rgb_lv_cnn_diff);                        //19
    offset = InfoBinGen(offset, str, "           \"age_group\": %d,\n",face_var->rgb_age_group);                                //20
    offset = InfoBinGen(offset, str, "           \"face_rgb_quality\": %d,\n",face_var->rgb_face_quality);                      //21
    offset = InfoBinGen(offset, str, "           \"face_nir_quality\": %d,\n",face_var->nir_face_quality);                      //22
    offset = InfoBinGen(offset, str, "           \"rgb_corner_y\": %d,\n",face_var->rgb_corner_y);                              //23
    offset = InfoBinGen(offset, str, "           \"effect_2d\": %f,\n",face_var->effect_2d);                                    //24
    offset = InfoBinGen(offset, str, "           \"sl_lv_cnn_real_score\": %f,\n",face_var->sl_lv_cnn_real_score);              //25
    offset = InfoBinGen(offset, str, "           \"rgb_init_env_luma\": %d,\n",face_var->rgb_avg_luma);                         //26
    offset = InfoBinGen(offset, str, "           \"rgb_face_l_luma\": %d,\n",face_var->rgb_face_l_luma);                        //27
    offset = InfoBinGen(offset, str, "           \"rgb_face_r_luma\": %d,\n",face_var->rgb_face_r_luma);                        //28
#if CFG_PALM_PRINT_MODE == 1
    u8 mode = kdp_is_palm_mode(); //reuse it for palm mode.
    offset = InfoBinGen(offset, str, "           \"rgb_led_lv_history_flag\": %d,\n", mode);        //29
#else
    offset = InfoBinGen(offset, str, "           \"rgb_led_lv_history_flag\": %d,\n",face_var->rgb_led_lv_history_flag);        //29
#endif
    offset = InfoBinGen(offset, str, "           \"nir_led_flag\": %d, \n",face_var->nir_led_flag);                             //30
    offset = InfoBinGen(offset, str, "           \"rgb_y_average\": %d,\n",face_var->y_average);  								//31
    offset = InfoBinGen(offset, str, "           \"rgb_init_exp_time\": %d,\n",face_var->rgb_init_exp_time);  					//32
    offset = InfoBinGen(offset, str, "           \"nir_lv_cnn_face_real_score\": %f,\n",face_var->nir_lv_cnn_face_real_score);  //33
    offset = InfoBinGen(offset, str, "           \"nir_skin_luma\": %d,\n",face_var->nir_skin_luma);                            //34
    offset = InfoBinGen(offset, str, "           \"nir_lv_hsn_neck_score\": %.3f,\n",face_var->nir_lv_hsn_neck_score);          //35
    offset = InfoBinGen(offset, str, "           \"nir_lv_hsn_edge_score\": %.3f,\n",face_var->nir_lv_hsn_edge_score);           //36
    offset = InfoBinGen(offset, str, "           \"init_tile\": %d,\n",face_var->init_tile);           //37
    offset = InfoBinGen(offset, str, "           \"darkness\": %.1f,\n",face_var->darkness);           //38
    offset = InfoBinGen(offset, str, "           \"face_pose_score\": %.6f\n",face_var->face_pose_cur_val);

    offset = InfoBinGen(offset, str, "     },\n");

#if 0
    offset = InfoBinGen(offset, str, "    \"RGB parameter\":\n");
    offset = InfoBinGen(offset, str, "     {\n");
    offset = InfoBinGen(offset, str, "        \"FD\":\n");
    offset = InfoBinGen(offset, str, "        {\n");
    offset = InfoBinGen(offset, str, "            \"x\": %d,\n",kdp_e2e_get_r1_fd()->xywh[0]);
    offset = InfoBinGen(offset, str, "            \"y\": %d,\n",kdp_e2e_get_r1_fd()->xywh[1]);
    offset = InfoBinGen(offset, str, "            \"w\": %d,\n",kdp_e2e_get_r1_fd()->xywh[2]);
    offset = InfoBinGen(offset, str, "            \"h\": %d\n",kdp_e2e_get_r1_fd()->xywh[3]);
    offset = InfoBinGen(offset, str, "        },\n");

    offset = InfoBinGen(offset, str, "        \"LM\":\n");
    offset = InfoBinGen(offset, str, "        [\n");
    for(i=0;i<5;i++){
    offset = InfoBinGen(offset, str, "            {\n");
    offset = InfoBinGen(offset, str, "                \"mark\": %d,\n",i);
    offset = InfoBinGen(offset, str, "                \"x\": %d,\n",kdp_e2e_get_r1_lm()->marks[i].x);
    offset = InfoBinGen(offset, str, "                \"y\": %d\n",kdp_e2e_get_r1_lm()->marks[i].y);
    if(i != 4)
    offset = InfoBinGen(offset, str, "            },\n");
    else if(i == 4)
    offset = InfoBinGen(offset, str, "            }\n");
    }
    offset = InfoBinGen(offset, str, "        ],\n");
#endif
#if 0
    offset = InfoBinGen(offset, str, "\"LM_S\":\n");
    offset = InfoBinGen(offset, str, "[\n");
    for(i=0;i<5;i++){
    offset = InfoBinGen(offset, str, "{\n");
    offset = InfoBinGen(offset, str, "\"mark\": %d,\n",i);
    offset = InfoBinGen(offset, str, "\"x\": %d,\n",kdp_e2e_get_r1_lm_s()->marks[i].x);
    offset = InfoBinGen(offset, str, "\"y\": %d\n",kdp_e2e_get_r1_lm_s()->marks[i].y);
    if(i != 4)
    offset = InfoBinGen(offset, str, "},\n");
    else if(i == 4)
    offset = InfoBinGen(offset, str, "}\n");
    }
    offset = InfoBinGen(offset, str, "],\n");
    offset = InfoBinGen(offset, str, "        \"Score\": %f \n",kdp_e2e_get_r1_lm()->score );
    offset = InfoBinGen(offset, str, "     },\n");
#endif

    offset = InfoBinGen(offset, str, "    \"NIR parameter\":\n");
    offset = InfoBinGen(offset, str, "     {\n");
    offset = InfoBinGen(offset, str, "        \"FD\":\n");
    offset = InfoBinGen(offset, str, "        {\n");
    offset = InfoBinGen(offset, str, "            \"x\": %d,\n",kdp_e2e_get_n1_fd()->xywh[0]);
    offset = InfoBinGen(offset, str, "            \"y\": %d,\n",kdp_e2e_get_n1_fd()->xywh[1]);
    offset = InfoBinGen(offset, str, "            \"w\": %d,\n",kdp_e2e_get_n1_fd()->xywh[2]);
    offset = InfoBinGen(offset, str, "            \"h\": %d\n",kdp_e2e_get_n1_fd()->xywh[3]);
    offset = InfoBinGen(offset, str, "        },\n");

    offset = InfoBinGen(offset, str, "        \"LM\":\n");
    offset = InfoBinGen(offset, str, "        [\n");
    for(i=0;i<5;i++){
    offset = InfoBinGen(offset, str, "            {\n");
    offset = InfoBinGen(offset, str, "                \"mark\": %d,\n",i);
    offset = InfoBinGen(offset, str, "                \"x\": %f,\n",kdp_e2e_get_n1_lm()->marks[i].x_f);
    offset = InfoBinGen(offset, str, "                \"y\": %f\n",kdp_e2e_get_n1_lm()->marks[i].y_f);
    if(i != 4)
    offset = InfoBinGen(offset, str, "            },\n");
    else if(i == 4)
    offset = InfoBinGen(offset, str, "            }\n");
    }
    offset = InfoBinGen(offset, str, "        ],\n");

    offset = InfoBinGen(offset, str, "\"LM_S\":\n");
    offset = InfoBinGen(offset, str, "[\n");
    for(i=0;i<5;i++){
    offset = InfoBinGen(offset, str, "{\n");
    offset = InfoBinGen(offset, str, "\"mark\": %d,\n",i);
    offset = InfoBinGen(offset, str, "\"x\": %d,\n",kdp_e2e_get_n1_lm_s()->marks[i].x);
    offset = InfoBinGen(offset, str, "\"y\": %d\n",kdp_e2e_get_n1_lm_s()->marks[i].y);
    if(i != 4)
    offset = InfoBinGen(offset, str, "},\n");
    else if(i == 4)
    offset = InfoBinGen(offset, str, "}\n");
    }
    offset = InfoBinGen(offset, str, "],\n");

    offset = InfoBinGen(offset, str, "        \"Score\": %f,\n",kdp_e2e_get_n1_lm()->score );
    
    //FR
    offset = InfoBinGen(offset, str, "        \"FR\":\n");
    offset = InfoBinGen(offset, str, "        [\n");
    
    struct fr_result_s* n1_fr = kdp_e2e_get_n1_fr();
    for(i=0;i<FM_SIZE-1;i++){
        offset = InfoBinGen(offset, str, "%.4f,\n", i, n1_fr->feature_map[i]);
    }
    offset = InfoBinGen(offset, str, "%.4f\n", i, n1_fr->feature_map[i]);
    
    offset = InfoBinGen(offset, str, "        ]\n");
    offset = InfoBinGen(offset, str, "     },\n");
    
    offset = InfoBinGen(offset, str, "     \"Result\":\n");
    offset = InfoBinGen(offset, str, "     {\n");
    offset = InfoBinGen(offset, str, "           \"e2e_ret\": \"0x%x\"\n",dp_draw_info.e2e_ret);

    offset = InfoBinGen(offset, str, "     }\n");

    offset = InfoBinGen(offset, str, "}\n");

}
#endif

//extern func
void kl520_api_snapshot_adv_init(int idx, uint32_t buf_size)
{
#if CFG_SNAPSHOT_ENABLE == 1
#else
    for(int i=0;i<SNAPSHOT_ADV_NUM;i++){
        if(face_add_addr[idx][i] == 0){
            face_add_addr[idx][i] = kdp_ddr_reserve(buf_size);
            dbg_msg_api("[Snapshot]adv_init addr: 0x%d, idx:%d, buf_size:%d", face_add_addr[idx][i], idx, buf_size);
        }
    }
#endif
}

#if ( CFG_COM_BUS_TYPE & COM_BUS_UART_MASK )
#if defined KID_SNAP_IMAGE || defined KID_KN_SNAP_IMAGE

int kl520_api_save_to_snap_ddr(int cam_idx, u32 addr, int size)
{
    u8 face_idx = g_snap_img_cnt;    
    kl520_api_snapshot_dma(face_add_addr[cam_idx][face_idx], addr, size);
    return 0;
}

int kl520_api_save_snap_img_addr(int cam_idx, int face_idx, int off_w, int off_h)
{
    u8* pSrc = (u8*)face_add_addr[cam_idx][face_idx];
    u8* pDst = (u8*)KDP_DDR_TEST_RGB_IMG_ADDR;
    
    if(pSrc == 0) return -1;

    for ( int i = 0; i < NIR_IMG_SOURCE_H; i += off_h )
    {
        int off = i * NIR_IMG_SOURCE_W;
        for ( int j = 0; j < NIR_IMG_SOURCE_W; j += off_w )
        {
            *(pDst++) = *(pSrc + j + off);
        }
    }
    
    int nx = (NIR_IMG_SOURCE_W - 1) / off_w + 1;
    int ny = (NIR_IMG_SOURCE_H - 1) / off_h + 1;

    return nx * ny;
}
#endif
#endif

#if CFG_USB_SIMTOOL == 1
void kl520_api_sim_write_json(void)
{
    kl520_api_snapshot_get_inf();
    kl520_api_snapshot_dma(KDP_DDR_TEST_INF_IMG_ADDR, snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);
}
#endif

static osMutexId_t mutex_snapshot_dm = NULL;
void kl520_api_snapshot_dma(u32 writeAddr, u32 readAddr, u32 len)
{
    if(mutex_snapshot_dm == NULL)
        mutex_snapshot_dm = osMutexNew(NULL);

    osMutexAcquire(mutex_snapshot_dm, osWaitForever);

    while (kdp_dma_is_ch_busy(AHBDMA_Channel2));

    kdp_dma_reset_ch(AHBDMA_Channel2);
    kdp_dma_clear_interrupt(AHBDMA_Channel2);
    kdp_dma_enable_dma_int();
    kdp_dma_init(0,0,0);

    while (kdp_dma_is_ch_busy(AHBDMA_Channel2));
    kdp_dma_normal_mode(AHBDMA_Channel2,(UINT32)(readAddr),(UINT32)(writeAddr), len ,1,1,2,0,0,2,0,0); //init DMA and wait TE signal
    kdp_dma_enable_ch(AHBDMA_Channel2);
    kdp_dma_wait_dma_int(AHBDMA_Channel2);
    kdp_dma_disable_ch(AHBDMA_Channel2);
    osMutexRelease(mutex_snapshot_dm);

}


#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2

static void kl520_api_snapshot_adv_shot_save(void);

//
u8 snapshot_adv_type_log = 0;
u32 snapshot_adv_mask = 0;
u8 snapshot_adv_select = 0;
bool snapshot_adv_mode = FALSE;

u32 snap_shot_buffer_addr = 0;

//local

//global
void kl520_api_snapshot_adv_select(u8 idx)
{
    char buf[256];
    snapshot_adv_select = idx;

    if(snapshot_adv_select == 0){
        dbg_msg_api("[Snapshot]adv verify no save");    /*0*/
    }
    else if(snapshot_adv_select == 1){
        dbg_msg_api("[Snapshot]adv verify liveness log mode");    /*0*/
    }
    else if(snapshot_adv_select == 2){
        dbg_msg_api("[Snapshot]adv verify un-liveness log mode");
    }
    else if(snapshot_adv_select == 3){
        dbg_msg_api("[Snapshot]Five face log mode");
    }
    else if(snapshot_adv_select == 4){
        dbg_msg_api("[Snapshot]Aleays shot log mode");
    }
    else if(snapshot_adv_select == 5){
        dbg_msg_console("adv select type log:");
        dbg_msg_console("(1)E2E_ERROR_N1_LV_MODEL log");
        dbg_msg_console("(2)E2E_ERROR_FU_LV_MODEL log");
        dbg_msg_console("(3)E2E_ERROR_CV_LV_MODEL log");
        dbg_msg_console("(4)E2E_ERROR_N1_LV_MODEL or E2E_ERROR_FU_LV_MODEL or E2E_ERROR_CV_LV_MODEL log");

        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_nocrlf("");
        snapshot_adv_type_log = atoi(strtok(buf, " \r\n\t"));
    }
    else if(snapshot_adv_select == 6){
        dbg_msg_console("set mask value:");

        kdp_gets(DEBUG_CONSOLE, buf);
        dbg_msg_nocrlf("");
        char *ptr;
        snapshot_adv_mask = strtol(strtok(buf, " \r\n\t"), &ptr, 16);
        dbg_msg_console("mask %#x", snapshot_adv_mask);
    }
    else if(snapshot_adv_select == 7){
        dbg_msg_api("[Snapshot]Aleays shot structure light log mode");    /*0*/
    }
	else if(snapshot_adv_select == 8){
        dbg_msg_console("[Snapshot]only backup when finished function: kl520_api_fdfr_element");
    }
	else if(snapshot_adv_select == 9){
        dbg_msg_console("[Snapshot]only use in uart&usb mode");
    }
    else{
        snapshot_adv_select = 0;
        dbg_msg_api("[Snapshot]err, set default 0");
    }
}

void kl520_api_snapshot_adv_mode(void)
{
    if( snapshot_adv_select == 0 )
    {
        snapshot_adv_mode = FALSE;
#if CFG_SNAPSHOT_ENABLE == 2
        g_SnapUsbBufCnt = 0;
        g_SnapShotCnt = 0;
        g_SnapReadCnt = 0;
#endif
    }
    else
    {
        snapshot_adv_mode = TRUE;
    }

    if(snapshot_adv_mode == TRUE){
        dbg_msg_api("[Snapshot]adv on");
    }
    else{
        dbg_msg_api("[Snapshot]adv off");
    }
}


void kl520_api_snapshot_adv_shot_cont(u32 e2e_ret_or_faceadd_type)
{
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();


    if(e2e_ret_or_faceadd_type == E2E_ENVIR || e2e_ret_or_faceadd_type == E2E_ERROR_R1_LM_MOTION_CHK || e2e_ret_or_faceadd_type == E2E_ERROR_N1_LM_MOTION_CHK)
    {
        return;
    }

    u8 adv_select = snapshot_adv_select;
    if (adv_select == 9)
    {
        if (FACE_MODE_ADD == m_face_mode)
        {
            adv_select = 3;
        }
        else if ((FACE_MODE_RECOGNITION == m_face_mode) || (FACE_MODE_LIVENESS == m_face_mode))
        {
            adv_select = 4;
        }
    }

    if(adv_select == 0){
        return;
    }
    else if(adv_select == 1){ /*attack*/
        if(e2e_ret_or_faceadd_type == E2E_OK)return;
    #ifdef SNAPSHOT_LOG_ENABLE
        else{dbg_msg_api("[Snapshot]NO REAL, SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);}
    #endif
    }
    else if(adv_select == 2){ /*real*/
        if(e2e_ret_or_faceadd_type != E2E_OK)return;
    #ifdef SNAPSHOT_LOG_ENABLE
        else{dbg_msg_api("[Snapshot]NO ATTACK SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);}
    #endif
    }
    else if(adv_select == 3){ /*Five face log*/
        kdp_e2e_prop* prop = kdp_e2e_prop_get_inst();
        kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
        bool bSkip = true;

        if(e2e_ret_or_faceadd_type == E2E_VERY_SPECIAL_CODE) {
            bSkip = false;
        }

        if(bSkip == true)
            return;
    }
    else if(adv_select == 4){
        //dbg_msg_api("[Snapshot]ALWASY SHOT");
    }
    else if(adv_select == 5){

        if(snapshot_adv_type_log == 1){
            if(e2e_ret_or_faceadd_type == E2E_ERROR_N1_LV_MODEL)
            {
                #ifdef SNAPSHOT_LOG_ENABLE
                dbg_msg_api("[Snapshot]E2E_ERROR_N1_LV_MODEL, SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);
                #endif
            }
            else{return;}
        }
        else if(snapshot_adv_type_log == 2){
            if(e2e_ret_or_faceadd_type == E2E_ERROR_FU_LV_MODEL)
            {
                #ifdef SNAPSHOT_LOG_ENABLE
                dbg_msg_api("[Snapshot]E2E_ERROR_FU_LV_MODEL, SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);
                #endif
            }
            else{return;}
        }
        else if(snapshot_adv_type_log == 3){
            if(e2e_ret_or_faceadd_type == E2E_ERROR_CV_LV_MODEL)
            {
                #ifdef SNAPSHOT_LOG_ENABLE
                dbg_msg_api("[Snapshot]E2E_ERROR_N1_LV_MODEL, SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);
                #endif
            }
            else{return;}
        }
        else if(snapshot_adv_type_log == 4){
            if (e2e_ret_or_faceadd_type == E2E_ERROR_N1_LV_MODEL
             || e2e_ret_or_faceadd_type == E2E_ERROR_FU_LV_MODEL
             || e2e_ret_or_faceadd_type == E2E_ERROR_CV_LV_MODEL
            )
            {
                #ifdef SNAPSHOT_LOG_ENABLE
                dbg_msg_api("[Snapshot]E2E_ERROR_N1_LV_MODEL or E2E_ERROR_FU_LV_MODEL or E2E_ERROR_CV_LV_MODEL, SO SHOT (idx = %d, e2e_ret = %d)",g_SnapShotCnt,e2e_ret_or_faceadd_type);
                #endif
            }
            else{return;}
        }
    }
    else if(adv_select == 6){
        if (e2e_ret_or_faceadd_type & snapshot_adv_mask)
        {
            return;
        }
    }
    else if(adv_select == 7){
        if(e2e_ret_or_faceadd_type != E2E_ERROR_N1_S_LV_MODEL)
            return;
    }
    else if(adv_select == 8){
        return;
    }
    else{
    #ifdef SNAPSHOT_LOG_ENABLE
        dbg_msg_api("[Snapshot]err snapshot_adv_select = %d", adv_select);
    #endif
        return;
    }

    kl520_api_snapshot_adv_shot_save();

}


void kl520_api_snapshot_adv_shot_5face(void)
{
    if(snapshot_adv_select == 0){
        return;
    }
    else if((snapshot_adv_select == 3) ||
            ((snapshot_adv_select == 9) && (m_face_mode == FACE_MODE_ADD)))
    { /*Five face log*/
        kl520_api_snapshot_adv_shot_save();
        return;
    }
    else{
        return;
    }
}

// do data cppy when finished liveness check
void kl520_api_snapshot_adv_shot_after_fdfr_element(void)
{
    if(snapshot_adv_select == 0){
        return;
    }
    else if(snapshot_adv_select == 8){
        kl520_api_snapshot_adv_shot_save();
        return;
    }
    else{
        return;
    }
}

#if CFG_USB_SIMTOOL == 1
static float _kl520_api_snapshot_par_decoder(u16 paraidx)
{
    float ret;
    u32 *pwdata;
    pwdata=(void*)(KDP_DDR_TEST_INF_IMG_ADDR+paraidx*4);
    memcpy(&ret, pwdata, sizeof(ret));
    return  ret;
}

void kl520_api_snapshot_inf_decoder(void)
{
    kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();
    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();
    kdp_e2e_settings* settings = kdp_e2e_settings_get_inst();

//    for(u16 i=0; i<32;i++){
//        dbg_msg_console("para[%d] = %f",i,_kl520_api_snapshot_par_decoder(i));
//    }

   if ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL)) {
        dbg_msg_console("Substitute Liveness parameter(alg) nir tuned:");
        vars->init_tile_flag = (kdp_nir_init_flag)_kl520_api_snapshot_par_decoder(SIM_INIT_TILE_FLAG);
        vars->init_exp_tile = (u8)_kl520_api_snapshot_par_decoder(SIM_INIT_EXP_TILE);
        vars->nir_mode = (kdp_nir_mode)_kl520_api_snapshot_par_decoder(SIM_NIR_MODE);
        vars->init_nir_gain = _kl520_api_snapshot_par_decoder(SIM_INIT_NIR_GAIN);
        vars->nir_gain = _kl520_api_snapshot_par_decoder(SIM_NIR_GAIN);
        vars->nir_cur_exp_time = (u16)_kl520_api_snapshot_par_decoder(SIM_NIR_CUR_EXP_TIME);
        vars->rgb_led_flag = (u8)_kl520_api_snapshot_par_decoder(SIM_RGB_LED_FLAG);
        vars->rgb_cur_exp_time = _kl520_api_snapshot_par_decoder(SIM_RGB_CUR_EXP_TIME);
        vars->rgb_led_lv_history_flag = _kl520_api_snapshot_par_decoder(SIM_RGB_LED_LV_HISTORY_FLAG);

        vars->init_tile = _kl520_api_snapshot_par_decoder(SIM_INIT_TILE); //init tile
       
#if (CFG_AI_TYPE == AI_TYPE_N1R1)

        vars->nir_led_flag = TRUE; //for sim, always on.
#if CFG_PALM_PRINT_MODE == 1
       //reuse this for palm mode
       if(vars->rgb_led_lv_history_flag) kdp_set_palm_mode(1);
       else kdp_set_palm_mode(0);
#else   
    if (vars->rgb_led_flag == FALSE && vars->rgb_led_lv_history_flag == FALSE)
        vars->rgb_led_flag = FALSE;
    else 
        vars->rgb_led_flag = TRUE;
#endif

#endif
        settings->registered_offsetX = _kl520_api_snapshot_par_decoder(SIM_REGISTERED_OFFSETX);
        settings->registered_offsetY = _kl520_api_snapshot_par_decoder(SIM_REGISTERED_OFFSETY);
        if(settings->registered_offsetX == 0 && settings->registered_offsetY == 0){
            settings->calibration_count = 0;
        }
        else{
            settings->calibration_count = 1;
        }

        if(CFG_BOARD_PARAMS_0_ID != (u8)_kl520_api_snapshot_par_decoder(SIM_CFG_BOARD_PARAMS_0_ID)) {
            dbg_msg_console("[Sim Warning] CFG_BOARD_PARAMS_0_ID Sim and Inf are different: %d.", 
                (u8)_kl520_api_snapshot_par_decoder(SIM_CFG_BOARD_PARAMS_0_ID));
        }

   }
}
#endif

u32 kl520_api_snapshot_ddr_addr(int cam_idx)
{
    if(cam_idx == MIPI_CAM_NIR){   /*NIR*/
        return KDP_DDR_TEST_NIR_IMG_ADDR;
    }
    else if(cam_idx == MIPI_CAM_RGB){   /*RGB*/
        return KDP_DDR_TEST_RGB_IMG_ADDR;
    }
    else if(cam_idx == MIPI_CAM_INF){   /*INF*/
        return KDP_DDR_TEST_INF_IMG_ADDR;
    }
    else{
        return KDP_DDR_TEST_RGB_IMG_ADDR;
    }
}

#endif

#if CFG_SNAPSHOT_ENABLE == 1

void kl520_api_snapshot_init(void )
{
    int size = KDP_DDR_TEST_NIR_IMG_SIZE + KDP_DDR_TEST_RGB_IMG_SIZE + KDP_DDR_TEST_INF_IMG_SIZE;
    snap_shot_buffer_addr = kdp_ddr_reserve(size);
    
    snapshot_addr[MIPI_CAM_NIR] = snap_shot_buffer_addr + KDP_DDR_TEST_RGB_IMG_SIZE;
    snapshot_addr[MIPI_CAM_RGB] = snap_shot_buffer_addr;
    snapshot_addr[MIPI_CAM_INF] = snap_shot_buffer_addr + KDP_DDR_TEST_RGB_IMG_SIZE + KDP_DDR_TEST_NIR_IMG_SIZE;

    snapshot_status[MIPI_CAM_RGB] = 0;
    snapshot_status[MIPI_CAM_NIR] = 0;
    snapshot_status[MIPI_CAM_INF] = 0;
    if(snap_sts_mtx == NULL) {
        snap_sts_mtx = osMutexNew(NULL);
    }
    
    if (NULL == snapshot_evt) {
        snapshot_evt = osEventFlagsNew(NULL);
    }
}

int kl520_api_snapshot_fdfr_cam(int cam_idx, u32 addr)
{
    if(snapshot_adv_mode && (snapshot_adv_select != 0) ) {
        osMutexAcquire(snap_sts_mtx, osWaitForever); //protect

        //check busy.
        if(snapshot_status[MIPI_CAM_RGB] == 88) { //transferring
            osMutexRelease(snap_sts_mtx);
            return 0;
        }
        
        //save frame
        if(cam_idx == MIPI_CAM_NIR){   /*NIR*/
            snapshot_status[MIPI_CAM_NIR] = 0;
            snapshot_status[MIPI_CAM_INF] = 0; //make it invalid
            kl520_api_snapshot_dma(snapshot_addr[MIPI_CAM_NIR], addr, KDP_DDR_TEST_NIR_IMG_SIZE);
            snapshot_status[MIPI_CAM_NIR] = 1; //captured
        }
        if(cam_idx == MIPI_CAM_RGB){  /*RGB*/
            snapshot_status[MIPI_CAM_RGB] = 0;
            snapshot_status[MIPI_CAM_INF] = 0; //make it invalid
            kl520_api_snapshot_dma(snapshot_addr[MIPI_CAM_RGB], addr, KDP_DDR_TEST_RGB_IMG_SIZE);
            snapshot_status[MIPI_CAM_RGB] = 1; //captured
        }
        osMutexRelease(snap_sts_mtx);
    }
    return 0;
}

static int kl520_api_snapshot_fdfr_inf(int flag)
{
     if(flag) {
         kl520_api_snapshot_get_inf();
     }
    return 1;
}

static void kl520_api_snapshot_adv_shot_save(void)
{
    osMutexAcquire(snap_sts_mtx, osWaitForever); //protect

    //check busy.
    if(snapshot_status[MIPI_CAM_RGB] == 88) { //transferring
        osMutexRelease(snap_sts_mtx);
        return;
    }
    
#if CFG_PALM_PRINT_MODE == 1
    if(kdp_is_palm_mode()) {
        if(snapshot_status[MIPI_CAM_RGB] != 1) {
            osMutexRelease(snap_sts_mtx);
            return;
        } else {
            snapshot_status[MIPI_CAM_NIR] = 1; //make nir empty
        }
    }
    else
#endif
    {
#if (CFG_CAMERA_SINGLE_1054 == NO)
        //info file
        if(snapshot_status[MIPI_CAM_RGB] != 1 || snapshot_status[MIPI_CAM_NIR] != 1) {
            osMutexRelease(snap_sts_mtx);
            return;
        }
#else
        if(snapshot_status[MIPI_CAM_NIR] != 1) {
            osMutexRelease(snap_sts_mtx);
            return;
        } else {
            snapshot_status[MIPI_CAM_RGB] = 1; //make rgb empty
        }
#endif
    }

    kl520_api_snapshot_fdfr_inf(1);
    snapshot_status[MIPI_CAM_INF] = 1; //captured
    
    //notify usb thrd
    osEventFlagsSet(snapshot_evt, SNAP_SHOT_EVENT_READY);

    osMutexRelease(snap_sts_mtx);
    //dbg_msg_algo("img captured.");
    osDelay(10);
    return;
}

u32 kl520_api_snapshot_addr(int cam_idx)
{
    return kl520_api_snapshot_ddr_addr(cam_idx);   //buffer not ready
}

#endif

#if CFG_SNAPSHOT_ENABLE == 2

#define SNAPSHOT_FORMAT_RAW8    (0)
#define SNAPSHOT_FORMAT_RGB565  (1)

#if (CFG_AI_TYPE == AI_TYPE_N1)
#define SNAPSHOT_FORMAT_USING   (SNAPSHOT_FORMAT_RAW8) 
#else
#define SNAPSHOT_FORMAT_USING   (SNAPSHOT_FORMAT_RGB565) 
#endif

u8 g_SnapUsbBufCnt = 0;
u8 g_SnapShotCnt = 0;
u8 g_SnapReadCnt = 0;

static u32 snapshot_addr_back[SNAPSHOT_SRC_NUM]; //CFG_FMAP_EX_FIG_ENABLE

static u8 g_catch_idx[SNAPSHOT_SRC_NUM]={0};
static u8 g_catch_enable[SNAPSHOT_SRC_NUM]={0};

static int kl520_api_snapshot_fdfr_inf(int flag)
{
//    if( kl520_api_fdfr_exist_thread() && (flag == 0) )
//    {
//        if( (g_catch_idx[MIPI_CAM_RGB] < 2 && g_catch_enable[MIPI_CAM_RGB] == 1) &&
//            (g_catch_idx[MIPI_CAM_NIR] < 2 && g_catch_enable[MIPI_CAM_NIR] == 1) )
//            return 0;
//    }
    //rgb/nir ready
//    dbg_msg_api("g_catch_idx[MIPI_CAM_RGB] = %d",g_catch_idx[MIPI_CAM_RGB]);
//    dbg_msg_api("g_catch_idx[MIPI_CAM_NIR] = %d",g_catch_idx[MIPI_CAM_NIR]);
//    dbg_msg_api("g_catch_idx[MIPI_CAM_INF] = %d",g_catch_idx[MIPI_CAM_INF]);

#if CFG_SNAPSHOT_INFO == YES
     if(flag) {
         kl520_api_snapshot_get_inf();
     }
#endif

    if(g_catch_enable[MIPI_CAM_RGB] == 1)
        g_catch_idx[MIPI_CAM_RGB] = 0;

    if(g_catch_enable[MIPI_CAM_NIR] == 1)
        g_catch_idx[MIPI_CAM_NIR] = 0;

    if(g_catch_enable[MIPI_CAM_INF] == 1)
        g_catch_idx[MIPI_CAM_INF] = 0;

    return 1;
}

static void kl520_api_snapshot_adv_shot(int idx, u8 face_idx)
{
    if(face_idx >= SNAPSHOT_ADV_NUM){
        return;
    }

    if(snapshot_adv_mode == FALSE){
        return;
    }

    // jim memcpy data to buffer
    if( snapshot_adv_select == 8 )
    {

        if(kl520_api_fdfr_exist_thread())
            g_catch_idx[idx]++;

        if(idx == MIPI_CAM_NIR && face_add_addr[idx][face_idx] != 0){   /*NIR*/
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr_back[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot nir...idx:%d, face_idx:%d",idx,face_idx);

        }
        else if(idx == MIPI_CAM_RGB && face_add_addr[idx][face_idx] != 0){  /*RGB*/
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr_back[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot rgb...idx:%d, face_idx:%d",idx,face_idx);

        }
        else if(idx == MIPI_CAM_INF && face_add_addr[idx][face_idx] != 0){  /*INF*/

            kl520_api_snapshot_fdfr_inf(1);
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot inf...idx:%d, face_idx:%d",idx,face_idx);
        }
        else
        {
            dbg_msg_api("[Snapshot]adv_shot err");
        }
    }
    else
    {
        if(idx == MIPI_CAM_NIR && face_add_addr[idx][face_idx] != 0){   /*NIR*/
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot nir...idx:%d, face_idx:%d",idx,face_idx);

        }
        else if(idx == MIPI_CAM_RGB && face_add_addr[idx][face_idx] != 0){  /*RGB*/
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot rgb...idx:%d, face_idx:%d",idx,face_idx);

        }
        else if(idx == MIPI_CAM_INF && face_add_addr[idx][face_idx] != 0){  /*INF*/

            kl520_api_snapshot_fdfr_inf(1);
            //memcpy((void *)face_add_addr[idx][face_idx], (void *)snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);
            kl520_api_snapshot_dma(face_add_addr[idx][face_idx], snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);
            //dbg_msg_api("[Snapshot]adv_shot inf...idx:%d, face_idx:%d",idx,face_idx);
        }
        else
        {
            dbg_msg_api("[Snapshot]adv_shot err");
        }
    }
}

void kl520_api_snapshot_init(void )
{
    snapshot_addr[MIPI_CAM_NIR] = kdp_ddr_reserve(KDP_DDR_TEST_NIR_IMG_SIZE);;
    snapshot_addr[MIPI_CAM_RGB] = kdp_ddr_reserve(KDP_DDR_TEST_RGB_IMG_SIZE);;
    snapshot_addr[MIPI_CAM_INF] = kdp_ddr_reserve(KDP_DDR_TEST_INF_IMG_SIZE);;
    g_catch_enable[MIPI_CAM_INF] = 1;

#if (CFG_AI_TYPE == AI_TYPE_R1)
    g_catch_enable[MIPI_CAM_RGB] = 1;
#elif (CFG_AI_TYPE == AI_TYPE_R1N1)
    g_catch_enable[MIPI_CAM_RGB] = 1;
    g_catch_enable[MIPI_CAM_NIR] = 1;
#elif (CFG_AI_TYPE == AI_TYPE_N1)
    g_catch_enable[MIPI_CAM_NIR] = 1;
#elif (CFG_AI_TYPE == AI_TYPE_N1R1)
    g_catch_enable[MIPI_CAM_NIR] = 1;
    g_catch_enable[MIPI_CAM_RGB] = 1;
#endif

}

int kl520_api_snapshot_fdfr_cam(int cam_idx, u32 addr)
{

    kdp_e2e_face_variables *vars = kdp_e2e_get_face_variables();

    if(kl520_api_fdfr_exist_thread())
    {
        if(g_catch_enable[cam_idx] == 0)
            return -1;

        if( g_catch_idx[cam_idx] == 0)
            return -1;
    }
    // jim only backup not memcopy
    if( snapshot_adv_select == 8)
    {
        snapshot_addr_back[cam_idx] = addr;
    }
    else
    {
        if(cam_idx == MIPI_CAM_NIR){   /*NIR*/
            //memcpy((void *)snapshot_addr[MIPI_CAM_NIR], (void *)addr, KDP_DDR_TEST_NIR_IMG_SIZE);
            kl520_api_snapshot_dma(snapshot_addr[MIPI_CAM_NIR], addr, KDP_DDR_TEST_NIR_IMG_SIZE);
        }
        else if(cam_idx == MIPI_CAM_RGB){  /*RGB*/
            //memcpy((void *)snapshot_addr[MIPI_CAM_RGB], (void *)addr, KDP_DDR_TEST_RGB_IMG_SIZE);
            kl520_api_snapshot_dma(snapshot_addr[MIPI_CAM_RGB], addr, KDP_DDR_TEST_RGB_IMG_SIZE);
        }

        if(kl520_api_fdfr_exist_thread())
            g_catch_idx[cam_idx]++;
    }

    return 0;
}

static void kl520_api_snapshot_adv_shot_save(void)
{
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
    if(g_SnapUsbBufCnt < (SNAPSHOT_ADV_NUM))
#endif
    {
        kl520_api_snapshot_adv_shot(MIPI_CAM_NIR,g_SnapShotCnt);
        kl520_api_snapshot_adv_shot(MIPI_CAM_RGB,g_SnapShotCnt);
        kl520_api_snapshot_adv_shot(MIPI_CAM_INF,g_SnapShotCnt);

        g_SnapShotCnt++;
        g_SnapShotCnt = g_SnapShotCnt%SNAPSHOT_ADV_NUM;
    
#if ( CFG_COM_BUS_TYPE&COM_BUS_UART_MASK ) || ( CFG_COM_BUS_TYPE&COM_BUS_USB_MASK )
        g_SnapUsbBufCnt++;
        dbg_msg_console("[Snapshot]g_SnapShotCnt :%d, g_SnapUsbBufCnt:%d",g_SnapShotCnt, g_SnapUsbBufCnt);
        if(g_SnapUsbBufCnt == (SNAPSHOT_ADV_NUM))
            dbg_msg_api("[Snapshot]snapshot buffer FULL ");
#else
        dbg_msg_api("[Snapshot]g_SnapShotCnt :%d",g_SnapShotCnt);
#endif


    }
}


u32 kl520_api_snapshot_addr(int cam_idx)
{

    if(kl520_api_fdfr_exist_thread())
    {
        if( (g_catch_idx[MIPI_CAM_NIR] > 0 && g_catch_enable[MIPI_CAM_NIR] == 1) &&
            (g_catch_idx[MIPI_CAM_RGB] > 0 && g_catch_enable[MIPI_CAM_RGB] == 1) &&
            (g_catch_idx[MIPI_CAM_INF] > 0 && g_catch_enable[MIPI_CAM_INF] == 1) )
        {
            return 0;   //buffer not ready
        }
    }

    //memcpy((void *)KDP_DDR_TEST_NIR_IMG_ADDR, (void *)snapshot_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
    //memcpy((void *)KDP_DDR_TEST_RGB_IMG_ADDR, (void *)snapshot_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
    //memcpy((void *)KDP_DDR_TEST_INF_IMG_ADDR, (void *)snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);

    kl520_api_snapshot_dma(KDP_DDR_TEST_NIR_IMG_ADDR, snapshot_addr[MIPI_CAM_NIR], KDP_DDR_TEST_NIR_IMG_SIZE);
    kl520_api_snapshot_dma(KDP_DDR_TEST_RGB_IMG_ADDR, snapshot_addr[MIPI_CAM_RGB], KDP_DDR_TEST_RGB_IMG_SIZE);
    kl520_api_snapshot_dma(KDP_DDR_TEST_INF_IMG_ADDR, snapshot_addr[MIPI_CAM_INF], KDP_DDR_TEST_INF_IMG_SIZE);

    return kl520_api_snapshot_ddr_addr(cam_idx);   //buffer not ready
}

int kl520_api_snapshot_fdfr_catch(int cam_idx)
{
    if(kl520_api_fdfr_exist_thread())
    {
        if( (g_catch_idx[MIPI_CAM_NIR] > 0 && g_catch_enable[MIPI_CAM_NIR] == 1) &&
            (g_catch_idx[MIPI_CAM_RGB] > 0 && g_catch_enable[MIPI_CAM_RGB] == 1) &&
            (g_catch_idx[MIPI_CAM_INF] > 0 && g_catch_enable[MIPI_CAM_INF] == 1) )
        {
            return 0;   //catch buffer not ready
        }
    }

    if(g_catch_enable[MIPI_CAM_RGB] == 1)
        g_catch_idx[MIPI_CAM_RGB] = 1;

    if(g_catch_enable[MIPI_CAM_NIR] == 1)
        g_catch_idx[MIPI_CAM_NIR] = 1;

    if(g_catch_enable[MIPI_CAM_INF] == 1)
        g_catch_idx[MIPI_CAM_INF] = 1;

    return 1;

}

void kl520_api_ap_com_snapshot_offset_merge( void )
{
    u32 data_addr = KDP_DDR_TEST_RGB_IMG_ADDR;
    u32 src_0_img_size = kdp_e2e_get_img_mem_len(MIPI_CAM_RGB);
    u32 src_1_img_size = kdp_e2e_get_img_mem_len(MIPI_CAM_NIR);

    if( src_0_img_size !=  RGB_IMG_SOURCE_W*RGB_IMG_SOURCE_H*2 ) {
        data_addr+=src_0_img_size;
        kl520_api_snapshot_dma(data_addr, KDP_DDR_TEST_NIR_IMG_ADDR, src_1_img_size);
        data_addr+=src_1_img_size;
        kl520_api_snapshot_dma(data_addr, KDP_DDR_TEST_INF_IMG_ADDR, KDP_DDR_TEST_INF_IMG_SIZE);
    }
    else if( src_1_img_size !=  NIR_IMG_SOURCE_W*NIR_IMG_SOURCE_H ) {
        data_addr+=src_0_img_size;
        data_addr+=src_1_img_size;
        kl520_api_snapshot_dma(data_addr, KDP_DDR_TEST_INF_IMG_ADDR, KDP_DDR_TEST_INF_IMG_SIZE);
    }    

}

void kl520_api_snapshot_adv_chk(void)
{
    if(snapshot_adv_mode == TRUE){
        kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR);
        kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB);
    }
}

bool snapshot_cus_mode = FALSE;
u32 snap_dispaly_addr = 0;

int kl520_api_snapshot_enable(bool enable){

    snapshot_cus_mode = enable;
    return SNAPIMG_SUCCESS;
}

void kl520_api_snapshot_shot(u32 disp_addr){

#if SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RAW8
    u32 nLen = DISPLAY_WIDTH*DISPLAY_HEIGHT;
#elif (SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RGB565 )
    u32 nLen = DISPLAY_WIDTH*DISPLAY_HEIGHT*2;
#endif

    if(snapshot_cus_mode == FALSE)
        return;

    if(disp_addr == 0)
        return;

    if(snap_dispaly_addr == 0){
        snap_dispaly_addr = kdp_ddr_reserve(nLen);
    }


#if SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RAW8
    //l520_api_snapshot_dma(snap_dispaly_addr, disp_addr,  nLen);

    {
        u16 data_buf;
        //u16 data_buf1, data_buf2, data_buf3;
        u32 readAddr;
        u32 writeAddr;    

        readAddr =  disp_addr;
        writeAddr = snap_dispaly_addr;
        for(u32 i =0; i < DISPLAY_WIDTH*DISPLAY_HEIGHT; i++)
        {
            u8 data_out = 0;
            data_buf = (u16)inl(readAddr + (i* 2));
            //data_buf1 = data_buf;
            //data_buf3 = data_buf;
            //data_buf = (u16)((data_buf>>3)<<11) | ((data_buf>>2)<<5) | ((data_buf>>3)<<0);
            //data_out = ( data_out | ( (data_buf1 >> 11) & 0x1F )) << 3 ;
            data_out = ( data_out | ( (data_buf >> 5 ) & 0x3F )) << 2;
            //data_out = ( data_out | ( (data_buf3 >> 0 ) & 0x1F ));
            outb((writeAddr + i),  data_out);
        }
    }

#elif (SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RGB565 )
//    memcpy((void *)snap_dispaly_addr, (void *)disp_addr, nLen);
    kl520_api_snapshot_dma(snap_dispaly_addr, disp_addr,  nLen);
    
#endif
   
    dbg_msg_api("[SnapImage]Copy to temp.. ok");
    snapshot_cus_mode = FALSE;
}



void snapshot_record_monitor_delay_ctrl(u16 nDelayCnt)
{
    u16 nTimeCnt = 0;

    while((snapshot_cus_mode==TRUE) && nTimeCnt<nDelayCnt)
    {
        osDelay(10);
        nTimeCnt++;
    }
}

int _kl520_api_snapshot_username_decoder(u16 paraidx)
{
    char  ret;
    kdp_e2e_face_variables* vars = kdp_e2e_get_face_variables();
    u32 *pwdata;
    pwdata=(void*)(KDP_DDR_TEST_INF_IMG_ADDR+paraidx);
    memcpy( vars->user_name, pwdata, MAX_LEN_USERNAME );
    dbg_msg_console("Username : %s", vars->user_name);

    return  ret;
}

void kl520_api_snapshot_img_scale_down(u32 nSrcAddr, u32 nDstAddr, u16 nW, u16 nH, u16 nOffsetW, u16 nOffsetH, BOOL bU16Type)
{
    u32 i, j;

    if ( bU16Type )
    {
        u16 *pSrc;
        u16 *pDst;

        pSrc = (u16*)nSrcAddr;
        pDst = (u16*)nDstAddr;

        for ( i = 0; i < nW*nH; i+=nOffsetH )
        {
            for ( j = 0; j < nW; j+=nOffsetW )
            {
                *(pDst++) = *(pSrc+j+i);
            }
        }
    }
    else
    {
        u8* pSrc;
        u8* pDst;

        pSrc = (u8*)nSrcAddr;
        pDst = (u8*)nDstAddr;

        for ( i = 0; i < nW*nH; i+=nOffsetH )
        {
            for ( j = 0; j < nW; j+=nOffsetW )
            {
                *(pDst++) = *(pSrc+j+i);
            }
        }
    }
}

void kl520_api_snapshot_adv_load(int cam_idx, u8 face_idx)
{

    if(face_idx >= SNAPSHOT_ADV_NUM)
    {
        return;
    }

    if(cam_idx == MIPI_CAM_NIR && face_add_addr[cam_idx][face_idx] != 0){   /*NIR*/
#if ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV > 1 ) && ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV%2 == 0 )
        kl520_api_snapshot_img_scale_down(face_add_addr[cam_idx][face_idx],\
                                          KDP_DDR_TEST_NIR_IMG_ADDR,\
                                          NIR_IMG_SOURCE_W,\
                                          NIR_IMG_SOURCE_H,\
                                          CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV,\
                                          (NIR_IMG_SOURCE_W*CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV),\
                                          FALSE);
#else
        //memcpy((void *)KDP_DDR_TEST_NIR_IMG_ADDR, (void *)face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_NIR_IMG_SIZE);
        kl520_api_snapshot_dma(KDP_DDR_TEST_NIR_IMG_ADDR, face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_NIR_IMG_SIZE);
        //dbg_msg_api("[Snapshot]adv_load nir...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
#endif
    }
    else if(cam_idx == MIPI_CAM_RGB && face_add_addr[cam_idx][face_idx] != 0){  /*RGB*/
#if ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV > 1 ) && ( CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV%2 == 0 )
        kl520_api_snapshot_img_scale_down(face_add_addr[cam_idx][face_idx],\
                                          KDP_DDR_TEST_RGB_IMG_ADDR,\
                                          RGB_IMG_SOURCE_W,\
                                          RGB_IMG_SOURCE_H,\
                                          CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV,\
                                          (RGB_IMG_SOURCE_W*CUSTOMER_CATCH_EXP_IMG_SCALE_DOWN_DIV),\
                                          TRUE);
#else
        //memcpy((void *)KDP_DDR_TEST_RGB_IMG_ADDR, (void *)face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_RGB_IMG_SIZE);
        kl520_api_snapshot_dma(KDP_DDR_TEST_RGB_IMG_ADDR, face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_RGB_IMG_SIZE);
        //dbg_msg_api("[Snapshot]adv_load rgb...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
#endif
    }
    else if(cam_idx == MIPI_CAM_INF && face_add_addr[cam_idx][face_idx] != 0){  /*INF*/
        //memcpy((void *)KDP_DDR_TEST_INF_IMG_ADDR, (void *)face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_INF_IMG_SIZE);
        kl520_api_snapshot_dma(KDP_DDR_TEST_INF_IMG_ADDR, face_add_addr[cam_idx][face_idx], KDP_DDR_TEST_INF_IMG_SIZE);
        //dbg_msg_api("[Snapshot]adv_load inf...cam_idx:%d, face_idx:%d",cam_idx,face_idx);
    }
    else
    {
        dbg_msg_api("[Snapshot]adv_load err");
    }
}



u32 kl520_api_snapshot_addr_360X480()
{

    if(kl520_api_fdfr_exist_thread())
    {
        if( (g_catch_idx[MIPI_CAM_NIR] > 0 && g_catch_enable[MIPI_CAM_NIR] == 1) &&
            (g_catch_idx[MIPI_CAM_RGB] > 0 && g_catch_enable[MIPI_CAM_RGB] == 1) &&
            (g_catch_idx[MIPI_CAM_INF] > 0 && g_catch_enable[MIPI_CAM_INF] == 1) )
        {
            return 0;   //buffer not ready
        }
    }
    /*
    u16* p_dst;
    u16* p_src;

    p_dst = (u16*)KDP_DDR_TEST_RGB_IMG_ADDR;
    p_src = (u16*)(snapshot_addr[MIPI_CAM_RGB]+360);

    for( int i = 0; i < 480; i++ )
    {
        //dbg_msg_console("320x480!!!! run[%d]: 0x%08x, 0x%08x", i, p_dst, p_src );
        memcpy((void *)p_dst, (void *)p_src, 360*2);

        p_dst+=360;
        p_src+=640;
    }
    */

    dma_2d_memcpy_t info;

    info.src.addr = snapshot_addr[MIPI_CAM_RGB];
    info.src.start_x = 180;
    info.src.start_y = 0;
    info.src.w = 640;
    info.src.data_type = 2;

    info.dst.addr = KDP_DDR_TEST_RGB_IMG_ADDR;
    info.dst.start_x = 0;
    info.dst.start_y = 0;
    info.dst.w = 360;
    info.dst.data_type = 2;

    info.data_w = 360;
    info.data_h = 480;

    kdp_api_2D_memcpy( info );

   return KDP_DDR_TEST_RGB_IMG_ADDR;   //buffer not ready
}

u32 kl520_api_host_com_snpahot_status( void )
{
#if CFG_FMAP_EX_FIG_ENABLE == YES && CFG_FMAP_EXTRA_ENABLE == YES && CFG_FMAP_AP_CTRL_TYPE > EX_FM_DISABLE && CFG_FMAP_AP_CTRL_TYPE < EX_FM_UART_AP_CTRL_MAIN_DB
    return (u32)g_SnapUsbBufCnt | (KL520_api_ap_com_get_extra_fmap_status() << 16);  // and "len" parameter for firmware version
#else
    return (u32)g_SnapUsbBufCnt;  // and "len" parameter for firmware version
#endif
}

u32 kl520_api_ap_com_snapshot_catch( u16 src_type )
{

    u16 ret;
    u32 srt = osKernelGetTickCount();
    dbg_msg("[lwcom] get snapshot %#x cmd !!", src_type);
    ret = 1;//YES;

    //for( int i=0; i<*st_com->rx_buffer_index; i++ ) dbg_msg_console("spi rx buf[%d]=%#x", i, *(st_com->rx_buffer+i) );

    if(src_type == 0x5A)
    {
        while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB)==0)
        {osDelay(10);}
        while(kl520_api_snapshot_addr(MIPI_CAM_RGB)==0)
        {osDelay(10);}
    }
    else if(src_type == 0xA5)
    {
        while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_NIR)==0)
        {osDelay(10);}
        while(kl520_api_snapshot_addr(MIPI_CAM_NIR)==0)
        {osDelay(10);}
    }
    else if(src_type == 0xAA)
    {
        if(snapshot_adv_mode == TRUE)
        {
            if(g_SnapUsbBufCnt > 0)
            {
                kl520_api_snapshot_adv_load(MIPI_CAM_NIR, g_SnapReadCnt);
                kl520_api_snapshot_adv_load(MIPI_CAM_RGB, g_SnapReadCnt);
                kl520_api_snapshot_adv_load(MIPI_CAM_INF, g_SnapReadCnt);

                if(g_SnapUsbBufCnt>0)
                    g_SnapUsbBufCnt--;

                dbg_msg_com("[Snapshot]g_SnapReadCnt :%d, g_SnapUsbBufCnt:%d",g_SnapReadCnt, g_SnapUsbBufCnt);

                g_SnapReadCnt++;
                g_SnapReadCnt = g_SnapReadCnt%SNAPSHOT_ADV_NUM;

                if(g_SnapUsbBufCnt == 0)
                    dbg_msg_com("[Snapshot]Buffer NULL");
            }
        }
        else
        {
            while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB)==0)
            {osDelay(10);    }
            while(kl520_api_snapshot_addr(MIPI_CAM_RGB)==0)
            {osDelay(10);}
        }
        kl520_api_ap_com_snapshot_offset_merge();
        
    }
    else if(src_type == 0x015A)
    {
        while(kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB)==0)
        {osDelay(10);}
        while(kl520_api_snapshot_addr_360X480()==0)
        {osDelay(10);}
    }
    else
    {
        ret = 0;//NO;
        //dbg_msg("--> unknow");
    }

    dbg_msg_com("[lwcom] get snapshot result: %d!", ret);

    return ret;
}

u32 kl520_api_ap_com_snapshot_catch_addr( u16 src_type )
{
    if(src_type == 0x5A ){
        return KDP_DDR_TEST_RGB_IMG_ADDR;
    }
    else if(src_type == 0xA5 ){
        return KDP_DDR_TEST_NIR_IMG_ADDR;
    }
    else{
        return KDP_DDR_TEST_RGB_IMG_ADDR;
    }
}


#endif


#if (CFG_SNAPSHOT_ENABLE == 2) && (CFG_SNAPSHOT_ADVANCED == 1)

extern void sample_face_close(void);

int kl520_api_snapshot_get(u8 idx, kl520_snapshot_info *snapinfo)
{
    int ret = SNAPIMG_SUCCESS;
    u32 nLength = sizeof(kl520_snapshot_info);
    u32 nAddr = (u32)(KDP_FLASH_SNAPSHOT_IMG_00_ADDR + ((idx%CFG_SNAPSHOT_NUMS)*KDP_FLASH_SNAPSHOT_IMG_SIZE));

    kdp_memxfer_flash_to_ddr(KDP_DDR_DRV_SNAPSHOT_START_ADDR, nAddr, nLength);
    //dbg_msg_api("load img from flash to ddr...ok");

    memcpy(snapinfo, (void *)KDP_DDR_DRV_SNAPSHOT_START_ADDR, nLength);
    //dbg_msg_api("copy head form ddr to head...ok");

    if(snapinfo->nSanpIdx >= CFG_SNAPSHOT_NUMS)
    {
        //Overflow
        ret = SNAPIMG_IDX_OVERFLOW;
    }

    return ret;
}

u32 kl520_api_snapshot_data_addr(kl520_snapshot_info *snapinfo)
{
    u32 SnapDataAddr = 0;

    if(snapinfo->nSanpIdx >= CFG_SNAPSHOT_NUMS)
    {
        //Overflow
        SnapDataAddr = SNAPIMG_IDX_OVERFLOW;
    }
    else if ( (snapinfo->nInfoLen+snapinfo->nDataLen) > KDP_DDR_DRV_SNAPSHOT_RESERVED )
    {
        dbg_msg_api("[SnapImage] DDR buffer overflow...");
        SnapDataAddr = SNAPIMG_BUF_OVERFLOW;
    }
    else
    {
        dbg_msg_api("[SnapImage] Flash addr: 0x%x, Len: 0x%x",snapinfo->nDataAddr, snapinfo->nDataLen);
        SnapDataAddr = KDP_DDR_DRV_SNAPSHOT_START_ADDR+snapinfo->nInfoLen;
        kdp_memxfer_flash_to_ddr(SnapDataAddr, snapinfo->nDataAddr, snapinfo->nDataLen);
        dbg_msg_api("[SnapImage] Load img from flash to ddr...ok");
    }

    return SnapDataAddr;
}

u32 kl520_api_snapshot_falshIdx(void)
{
    u32 nSnapshotRecordIdx = 0;
    kl520_snapshot_info SnapInfo;
    u8 i;

    for(i=0;i<CFG_SNAPSHOT_NUMS;i++)
    {
        if(kl520_api_snapshot_get(i, &SnapInfo) != SNAPIMG_SUCCESS)
        {
            continue;
        }

        if(SnapInfo.nRecIdx >= nSnapshotRecordIdx)
        {
            nSnapshotRecordIdx = SnapInfo.nRecIdx;
        }
    }

    return nSnapshotRecordIdx;
}

u8 kl520_api_snapshot_recidx(void){

    u8 i;
    u8 idx;
    u32 nSnapshotRecordIdx = 0xFFFFFFFF;
    kl520_snapshot_info SnapInfo;

    for(i=0;i<CFG_SNAPSHOT_NUMS;i++)
    {
        /*Null*/
        if(kl520_api_snapshot_get(i, &SnapInfo) != SNAPIMG_SUCCESS)
        {
            idx = i;
            break;
        }

        /*Find nRecIdx min*/
        if(SnapInfo.nRecIdx <= nSnapshotRecordIdx)
        {
            nSnapshotRecordIdx = SnapInfo.nRecIdx;
            idx = i;
        }
    }

    dbg_msg_api("rec idx = %d", idx);

    return idx;
}

u32 kl520_api_snapshot_db(void)
{
    u32 nSnapshotFlashIdx = 0;
    kl520_snapshot_info SnapInfo;
    u8 i;
    dbg_msg_console("Idx|  RecIdx|      Addr|Length|DateTime");

    for(i=0;i<CFG_SNAPSHOT_NUMS;i++)
    {
        if(kl520_api_snapshot_get(i, &SnapInfo) != SNAPIMG_SUCCESS)
        {
            continue;
        }

        dbg_msg_console("%3d|%8d|0x%08X|%d|%llu",
        SnapInfo.nSanpIdx,
        SnapInfo.nRecIdx,
        SnapInfo.nInfoAddr,
        SnapInfo.nLength,
        SnapInfo.nTime);

        if(SnapInfo.nRecIdx >= nSnapshotFlashIdx)
        {
            nSnapshotFlashIdx = SnapInfo.nRecIdx;
        }
    }

//    dbg_msg_api("nSnapshotFlashIdx=%lu", nSnapshotFlashIdx);
    return nSnapshotFlashIdx;
}

int kl520_snapshot_delete(u8 idx){
    int ret = SNAPIMG_SUCCESS;
    kl520_snapshot_info SnapInfo;

    if(kl520_api_snapshot_get(idx, &SnapInfo) == SNAPIMG_SUCCESS)
    {
        SnapInfo.nSanpIdx = 0xFF;
        SnapInfo.nRecIdx = 0xFFFFFFFF;
        memcpy((void *)KDP_DDR_DRV_SNAPSHOT_START_ADDR, &SnapInfo, sizeof(SnapInfo));
        kdp_memxfer_ddr_to_flash((u32)SnapInfo.nInfoAddr, (u32)KDP_DDR_DRV_SNAPSHOT_START_ADDR, sizeof(SnapInfo));
    }
    else
    {
        ret = SNAPIMG_DEL_FAIL;
    }

    return ret;
}

int kl520_api_snapshot_delete(BOOL all ,u8 idx){

    if(all == TRUE){
        u8 i;
        for(i=0;i<CFG_SNAPSHOT_NUMS;i++){
            kl520_snapshot_delete(i);
        }
        dbg_msg_api("delete all");
    }
    else
    {
        kl520_snapshot_delete(idx);
    }

    kl520_api_snapshot_db();

    return SNAPIMG_SUCCESS;
}

int kl520_api_snapshot_del_img(u8 nDelIdx, u8 nNumCnt)
{
    int ret = SNAPIMG_SUCCESS;

    u8 i, nIdx = nDelIdx%CFG_SNAPSHOT_NUMS;

    for ( i = 0; i < nNumCnt; i++, nIdx++ )
    {
        if ( nIdx == CFG_SNAPSHOT_NUMS )
        {
            nIdx %= CFG_SNAPSHOT_NUMS;
        }

        kl520_snapshot_delete(nIdx);
    }

    return ret;
}

int kl520_api_snapshot_record(u8 idx, u64 time, u8 result)
{
    int ret = SNAPIMG_SUCCESS;

    kl520_snapshot_info SnapInfo;

    if ( idx >= CFG_SNAPSHOT_NUMS )
    {
        //Overflow
        dbg_msg_api("[SnapImage] Recode index overflow...");
        ret = SNAPIMG_IDX_OVERFLOW;
    }
    else
    {
        //Flash Init header
        if(kl520_api_snapshot_get(idx, &SnapInfo) != SNAPIMG_SUCCESS)
        {
            SnapInfo.nSanpIdx = idx;
            SnapInfo.nRecIdx = kl520_api_snapshot_falshIdx()+1;
            SnapInfo.nInfoAddr = (u32)(KDP_FLASH_SNAPSHOT_IMG_00_ADDR + (idx*KDP_FLASH_SNAPSHOT_IMG_SIZE));
            SnapInfo.nInfoLen = sizeof(SnapInfo);
            SnapInfo.nDataAddr = (SnapInfo.nInfoAddr + SnapInfo.nInfoLen);
#if SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RAW8 
            SnapInfo.nDataLen = DISPLAY_WIDTH*DISPLAY_HEIGHT;            
#elif (SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RGB565 )
            SnapInfo.nDataLen = (DISPLAY_WIDTH*DISPLAY_HEIGHT*2);
#endif
            SnapInfo.nWidth = DISPLAY_WIDTH;
            SnapInfo.nHeight = DISPLAY_HEIGHT;
            SnapInfo.nTime = time;
            SnapInfo.nLength = (SnapInfo.nInfoLen + SnapInfo.nDataLen);
            SnapInfo.nResult = result;

            //Flash Init header
            dbg_msg_api("Flash Init header...ok");

            dbg_msg_api("SnapInfo.nSanpIdx=%d",SnapInfo.nSanpIdx);
            dbg_msg_api("SnapInfo.nRecIdx=%lu",SnapInfo.nRecIdx);
            dbg_msg_api("SnapInfo.nInfoAddr=0x%x",SnapInfo.nInfoAddr);
            dbg_msg_api("SnapInfo.nInfoLen=%d",SnapInfo.nInfoLen);
            dbg_msg_api("SnapInfo.nDataAddr=0x%x",SnapInfo.nDataAddr);
            dbg_msg_api("SnapInfo.nDataLen=%d",SnapInfo.nDataLen);
            dbg_msg_api("SnapInfo.nWidth=%d",SnapInfo.nWidth);
            dbg_msg_api("SnapInfo.nHeight=%d",SnapInfo.nHeight);
            dbg_msg_api("SnapInfo.nTime=%llu",SnapInfo.nTime);
            dbg_msg_api("SnapInfo.nLength=%d",SnapInfo.nLength);
        }
        else
        {
            //Update
            dbg_msg_api("[SnapImage] Update Info...ok");
            SnapInfo.nRecIdx = kl520_api_snapshot_falshIdx()+1;
            SnapInfo.nTime = time;
            SnapInfo.nResult = result;
            
            SnapInfo.nDataAddr = (SnapInfo.nInfoAddr + SnapInfo.nInfoLen);
#if SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RAW8 
            SnapInfo.nDataLen = DISPLAY_WIDTH*DISPLAY_HEIGHT;            
#elif (SNAPSHOT_FORMAT_USING == SNAPSHOT_FORMAT_RGB565 )
            SnapInfo.nDataLen = (DISPLAY_WIDTH*DISPLAY_HEIGHT*2);
#endif
            SnapInfo.nWidth = DISPLAY_WIDTH;
            SnapInfo.nHeight = DISPLAY_HEIGHT;

            SnapInfo.nLength = (SnapInfo.nInfoLen + SnapInfo.nDataLen);
        }

        if ( (SnapInfo.nInfoLen+SnapInfo.nDataLen) > KDP_DDR_DRV_SNAPSHOT_RESERVED )
        {
            dbg_msg_api("[SnapImage] DDR buffer overflow...");
            ret = SNAPIMG_BUF_OVERFLOW;
        }
        else
        {
            memcpy((void *)KDP_DDR_DRV_SNAPSHOT_START_ADDR, &SnapInfo, SnapInfo.nInfoLen);

            time = 0;
            while(1)
            {
                if(snapshot_cus_mode == FALSE){
                    //memcpy((void *)(KDP_DDR_DRV_SNAPSHOT_START_ADDR+SnapInfo.nInfoLen), (void *)snap_dispaly_addr, SnapInfo.nDataLen);
                    kl520_api_snapshot_dma((u32)(KDP_DDR_DRV_SNAPSHOT_START_ADDR+SnapInfo.nInfoLen), (u32)snap_dispaly_addr,  SnapInfo.nDataLen);
                    kdp_memxfer_ddr_to_flash((u32)SnapInfo.nInfoAddr, (u32)KDP_DDR_DRV_SNAPSHOT_START_ADDR, SnapInfo.nLength);
                    break;
                }

                if(time > 200){
                    dbg_msg_api("[SnapImage] Record time out");
                    ret = SNAPIMG_TIMEOUT;
                    break;
                }
                else{
                    time++;
                    osDelay(10);
                }
            }
        }

        if ( ret == SNAPIMG_SUCCESS )
        {
            if ( SnapInfo.nRecIdx == kl520_api_snapshot_falshIdx() )
            {
                dbg_msg_api("[SnapImage] Flash write...succeed");
            }
            else
            {
                dbg_msg_api("[SnapImage] Flash write...fail");
                ret = SNAPIMG_WD_FAIL;
            }
        }
    }

    return ret;
}

int kl520_api_snapshot_record_img(u8 nRecIdx, u8 nNumCnt, u64 time, u8 result)
{
    int ret = SNAPIMG_SUCCESS;

    u8 i, nIdx = nRecIdx%CFG_SNAPSHOT_NUMS;

    if ( kl520_api_cam_state_get(kdp_video_renderer_get_idx()) == KDP_DEVICE_CAMERA_RUNNING )
    {
        for ( i = 0; i < nNumCnt; i++, nIdx++ )
        {
            if ( nIdx == CFG_SNAPSHOT_NUMS )
            {
                nIdx %= CFG_SNAPSHOT_NUMS;
            }

            kl520_api_snapshot_enable(TRUE);
            ret = kl520_api_snapshot_record(nIdx, time, result);

            if ( ret == SNAPIMG_SUCCESS )
            {
                dbg_msg_console("[SnapImage] Recode %2d image, succeed", nIdx);
            }
            else
            {
                dbg_msg_console("[SnapImage] Recode %2d image, fail", nIdx);
                break;
            }
        }
    }
    else
    {
        dbg_msg_console("[SnapImage] Camera is not ready", nIdx);
        ret = SNAPIMG_CAM_NULL;
    }

    return ret;
}

int kl520_api_snapshot_record_img_early(u8 nRecIdx, u8 nNumCnt, u64 time, u8 result)
{
    int ret = SNAPIMG_SUCCESS;

    u8 i, nIdx = nRecIdx%CFG_SNAPSHOT_NUMS;
    b_en_aec_only = true;
    //dbg_msg_console("[SnapImage] b_en_aec_only");

    //if ( kl520_api_cam_state_get(kdp_video_renderer_get_idx()) == KDP_DEVICE_CAMERA_RUNNING )
    {
        for ( i = 0; i < nNumCnt; i++, nIdx++ )
        {
            if ( nIdx == CFG_SNAPSHOT_NUMS )
            {
                nIdx %= CFG_SNAPSHOT_NUMS;
            }
//            kl520_api_face_recognition_set_timeout(0);
//            kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
            #if (CFG_AI_TYPE == AI_TYPE_N1)
            kdp_e2e_nir_led_open();
            #endif
            
            kl520_api_face_liveness_set_timeout(0);
            kl520_api_face_liveness(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

            kl520_api_snapshot_enable(TRUE);
            snapshot_record_monitor_delay_ctrl(50);

            if ( kl520_api_cam_state_get(kdp_video_renderer_get_idx()) == KDP_DEVICE_CAMERA_RUNNING )
            {
                sample_face_close();

                ret = kl520_api_snapshot_record(nIdx, time, result);
            }
            else
            {
                dbg_msg_console("[SnapImage] Camera is not ready", nIdx);
                ret = SNAPIMG_CAM_NULL;
            }
            
            if ( ret == SNAPIMG_SUCCESS )
            {
                dbg_msg_console("[SnapImage] Recode %2d image, succeed", nIdx);
            }
            else
            {
                dbg_msg_console("[SnapImage] Recode %2d image, fail", nIdx);
                break;
            }
        }
    }
//    else
//    {
//        dbg_msg_console("[SnapImage] Camera is not ready", nIdx);
//        ret = SNAPIMG_CAM_NULL;
//    }
    b_en_aec_only = false;
    return ret;
}

int kl520_api_snapshot_show(u8 idx){

    int ret = SNAPIMG_SUCCESS;
    kl520_snapshot_info SnapInfo;
    u32 SnapDataAddr = 0;

    kl520_api_disp_open_chk();

    if(kl520_api_snapshot_get(idx, &SnapInfo) != SNAPIMG_SUCCESS)
    {
        kl520_api_dp_fresh_bg(BLACK, 2);
        dbg_msg_console("[SnapImage] Index %2d, No image data...", idx);
        ret = SNAPIMG_IMG_NULL;
    }
    else
    {
        SnapDataAddr = kl520_api_snapshot_data_addr(&SnapInfo);

        if ( SnapDataAddr < SNAPIMG_STATE_NUM )
        {
            ret = SnapDataAddr;
        }
        else
        {
            dbg_msg_api("SnapInfo.nSanpIdx=%d",SnapInfo.nSanpIdx);
            dbg_msg_api("SnapInfo.nRecIdx=%d",SnapInfo.nRecIdx);
            dbg_msg_api("SnapInfo.nInfoAddr=0x%x",SnapInfo.nInfoAddr);
            dbg_msg_api("SnapInfo.nInfoLen=%d",SnapInfo.nInfoLen);
            dbg_msg_api("SnapInfo.nDataAddr=0x%x",SnapInfo.nDataAddr);
            dbg_msg_api("SnapInfo.nDataLen=%d",SnapInfo.nDataLen);
            dbg_msg_api("SnapInfo.nWidth=%d",SnapInfo.nWidth);
            dbg_msg_api("SnapInfo.nHeight=%d",SnapInfo.nHeight);
            dbg_msg_api("SnapInfo.nTime=%llu",SnapInfo.nTime);
            dbg_msg_api("SnapInfo.nLength=%d",SnapInfo.nLength);

            kl520_api_dp_draw_bitmap(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (void *)SnapDataAddr);

            kl520_api_dp_fresh();
            dbg_msg_console("[SnapImage] Show %2d image", idx);
        }
    }

    return ret;
}

int kl520_api_snapshot_show_img(u8 nShowIdx, u8 nNumCnt)
{
    int ret = SNAPIMG_SUCCESS;

    u8 i, nIdx = nShowIdx%CFG_SNAPSHOT_NUMS;

    for ( i = 0; i < nNumCnt; i++, nIdx++ )
    {
        if ( nIdx == CFG_SNAPSHOT_NUMS )
        {
            nIdx %= CFG_SNAPSHOT_NUMS;
        }

        ret = kl520_api_snapshot_show(nIdx);

        if ( ret == SNAPIMG_SUCCESS )
        {
            osDelay(500);
        }
    }

    return ret;
}


void kl520_api_snapshot_fdfr_save(void)
{
    kdp_memxfer_ddr_to_flash((u32)KDP_FLASH_NIR_ADDR, (u32)snapshot_addr[MIPI_CAM_NIR], KDP_FLASH_NIR_SIZE);
    dbg_msg_api("[Snapshot]nir image save flash...ok");
    kdp_memxfer_ddr_to_flash((u32)KDP_FLASH_RGB_ADDR, (u32)snapshot_addr[MIPI_CAM_RGB], KDP_FLASH_RGB_SIZE);
    dbg_msg_api("[Snapshot]rgb image save flash...ok");
    kdp_memxfer_ddr_to_flash((u32)KDP_FLASH_INF_ADDR, (u32)snapshot_addr[MIPI_CAM_INF], KDP_FLASH_INF_SIZE);
    dbg_msg_api("[Snapshot]inf image save flash...ok");
}

#endif


#if CFG_USB_EXPORT_STREAM_IMG == YES

#if CFG_USB_EXPORT_LIVENESS_RET == YES
static u8 _g_liveness_ret_status = 0;

static u8 kl520_api_usb_get_export_liveness_ret( void )
{
    return _g_liveness_ret_status;
}

void kl520_api_usb_set_export_liveness_ret( u8 in )
{
    _g_liveness_ret_status = in;
}

u32 kl520_api_export_stream_get_fd_box( void )
{
    static u32 addr_bbox = 0;

    if( addr_bbox == 0 )
        addr_bbox = kdp_ddr_reserve( sizeof(int32_t) * 8 + sizeof(float)*2 + 4);

    // put your info strcut here

    if( addr_bbox != 0 )
    {
        int32_t *p;

        p = (int32_t*)addr_bbox;
        *p++ = kdp_e2e_get_r1_fd()->xywh[0];
        *p++ = kdp_e2e_get_r1_fd()->xywh[1];
        *p++ = kdp_e2e_get_r1_fd()->xywh[2];
        *p++ = kdp_e2e_get_r1_fd()->xywh[3];

        *p++ = kdp_e2e_get_n1_fd()->xywh[0];
        *p++ = kdp_e2e_get_n1_fd()->xywh[1];
        *p++ = kdp_e2e_get_n1_fd()->xywh[2];
        *p++ = kdp_e2e_get_n1_fd()->xywh[3];

        kdp_e2e_face_variables *face_var = kdp_e2e_get_face_variables();

        float *p2 = (float*)(addr_bbox+32);
        
        *p2++ = face_var->nir_gain;
        *p2++ = face_var->nir_cur_exp_time;

        
        *((u8*)(addr_bbox+32+8)) = kl520_api_usb_get_export_liveness_ret();
    /*
        p = addr_bbox;

        for( int i=0; i < 8; i++ )
        {
            dbg_msg_console("show bbox %d\n", *p++ );
        }
        dbg_msg_console("liveness_ret = %d", *((u8*)(addr_bbox+32)) );
    */

    }
    return addr_bbox;

}

#endif

int g_export_idx[IMGSRC_NUM]={0};
static s8 _g_export_image_ctrl = 0xFF;
static eSTREAM_IMAGE_EXPORT_SRC _g_export_rx_ctrl_idx = STRAM_IMAGE_TOTAL_SIZE_e;

void kl520_api_export_stream_get_info( u32 para, s8 *cam_idx, eSTREAM_IMAGE_EXPORT_SRC *export_mode )
{
    if( (para & 0x00FF) == 0x005A )
    {
        *cam_idx = MIPI_CAM_RGB;
    }
    else if( (para & 0x00FF) == 0x00A5 )
    {
        *cam_idx = MIPI_CAM_NIR;
    }
    else //if ( (msgcmd_arg->param1 & 0x00FF) == 0x00AA )
    {
        *cam_idx = IMGSRC_NUM;
    }


    if( para == 0x005A )
    {
        *export_mode = STRAM_IMAGE_RGB_640x480_e;
    }
    else if( para == 0x015A )
    {
        *export_mode = STRAM_IMAGE_RGB_320x480_e;
    }
    else if ( para == 0x025A )
    {
        *export_mode = STRAM_IMAGE_RGB_320x240_e;
    }
    else if ( para == 0x00A5 )
    {
        *export_mode = STRAM_IMAGE_NIR_480x640_e;
    }
    else if ( para == 0x00AA )
    {
        *export_mode = STRAM_IMAGE_BOTH_CAMERA_e;
    }
    else if ( para == 0x5A5A )
    {
        *export_mode = STRAM_IMAGE_DISPALY_e;
    }

}
s8 kl520_api_export_stream_get_image_crtl( void )
{
    return _g_export_image_ctrl;
}

eSTREAM_IMAGE_EXPORT_SRC kl520_api_export_stream_get_image_export_crtl( void )
{
    return _g_export_rx_ctrl_idx;
}

void kl520_api_export_stream_set_image_crtl( s8 in, eSTREAM_IMAGE_EXPORT_SRC export_src_idx )
{
    _g_export_image_ctrl = in;
    _g_export_rx_ctrl_idx = export_src_idx;
}


u32 kl520_api_export_stream_image_ddr_addr(eSTREAM_IMAGE_EXPORT_SRC cam_idx)
{
    if(cam_idx == STRAM_IMAGE_NIR_480x640_e ){   /*NIR*/
        return snapshot_addr[MIPI_CAM_NIR];
    }
    else if(cam_idx == STRAM_IMAGE_RGB_640x480_e ){   /*RGB*/
        return snapshot_addr[MIPI_CAM_RGB];
    }
    else{
        return KDP_DDR_TEST_RGB_IMG_ADDR;
    }
}


int kl520_api_export_stream_image_catch( void )
{
    if ( kl520_api_cam_to_dp_thread_alive() )
    {
        if( (g_export_idx[MIPI_CAM_NIR] > 0 && g_export_idx[MIPI_CAM_RGB] > 0  ) )
        {
            return 0;   //catch buffer not ready
        }
    }

    g_export_idx[MIPI_CAM_NIR] = 1;
    g_export_idx[MIPI_CAM_RGB] = 1;

    return 1;

}

u32 kl520_api_export_stream_image_addr( eSTREAM_IMAGE_EXPORT_SRC export_src_idx )
{

    if ( kl520_api_cam_to_dp_thread_alive() )
    {
        if( g_export_idx[MIPI_CAM_NIR] > 0 && g_export_idx[MIPI_CAM_RGB] > 0 )
        {
            return 0;   //buffer not ready
        }
    }

    return kl520_api_export_stream_image_ddr_addr(export_src_idx);   //buffer not ready

}

int kl520_api_export_stream_image(int cam_idx, u32 addr, eSTREAM_IMAGE_EXPORT_SRC export_src_idx )
{
    if ( kl520_api_cam_to_dp_thread_alive() )
    {
        if( g_export_idx[cam_idx] == 0) return -1;
    }

    if( export_src_idx == STRAM_IMAGE_DISPALY_e )
    {
        dbg_msg_console("displuy addr: %#X",addr);
        kl520_api_snapshot_dma(KDP_DDR_TEST_RGB_IMG_ADDR, addr, DISPLAY_HEIGHT*DISPLAY_WIDTH*2);
    }
    else
    {

        if(cam_idx == MIPI_CAM_NIR) /*NIR*/
        {
            snapshot_addr[cam_idx] = addr;

            if( export_src_idx == STRAM_IMAGE_BOTH_CAMERA_e )
            {
                kl520_api_snapshot_dma(KDP_DDR_TEST_NIR_IMG_ADDR, addr, KDP_DDR_TEST_NIR_IMG_SIZE);
            }

        }
        else if(cam_idx == MIPI_CAM_RGB) /*RGB*/
        {
            snapshot_addr[cam_idx] = addr;

            if( export_src_idx == STRAM_IMAGE_BOTH_CAMERA_e )
            {
                kl520_api_snapshot_dma(KDP_DDR_TEST_RGB_IMG_ADDR, addr, KDP_DDR_TEST_RGB_IMG_SIZE);
            }
            else if( export_src_idx == STRAM_IMAGE_RGB_320x480_e )
            {
                u16* p_dst;
                u16* p_src;

                p_dst = (u16*)KDP_DDR_TEST_RGB_IMG_ADDR;
                p_src = (u16*)(snapshot_addr[cam_idx]+320);

                for( int i = 0; i < 480; i++ )
                {
                    //dbg_msg_console("320x480!!!! run[%d]: 0x%08x, 0x%08x", i, p_dst, p_src );
                    memcpy((void *)p_dst, (void *)p_src, 320*2);

                    p_dst+=320;
                    p_src+=640;
                }
            }
            else if( export_src_idx == STRAM_IMAGE_RGB_320x240_e )
            {
                u16* p_dst;
                u16* p_src;
                p_dst = (u16*)KDP_DDR_TEST_RGB_IMG_ADDR;
                p_src = (u16*)snapshot_addr[cam_idx];

                for( int i = 0; i < 480*640; i+=1280 )
                {
                    for( int j = 0; j < 640; j+=2 )
                    {
                        *(p_dst++) = *(p_src+j+i);
                    }
                }
            }
        }
    }
    if ( kl520_api_cam_to_dp_thread_alive() )
        g_export_idx[cam_idx]++;

    return 0;
}


int kl520_api_export_stream_image_ready(void)
{
    if ( kl520_api_cam_to_dp_thread_alive() )
    {
        if( g_export_idx[MIPI_CAM_RGB] < 2 && g_export_idx[MIPI_CAM_NIR] < 2 )
            return 0;
    }

    g_export_idx[MIPI_CAM_RGB] = 0;
    g_export_idx[MIPI_CAM_NIR] = 0;

    return 1;
}
#endif //CFG_USB_EXPORT_STREAM_IMG == YES



