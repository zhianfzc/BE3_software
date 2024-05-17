#ifndef __KL520_API_CAMERA_H__
#define __KL520_API_CAMERA_H__


#include "kdp_camera.h"

#define MODIFY_CAM_DISP_CT_FLOW

//Permanent Control
#define API_CTRL_CAM_OPEN       (0x30)
#define API_CTRL_CAM_CLOS       (0x20)
#define API_CTRL_CAM_STAR       (0x0C)
#define API_CTRL_CAM_STOP       (0x08)
#define API_CTRL_DISP_OPEN      (0xC0)  //DISP: Display
#define API_CTRL_DISP_CLOS      (0x80)

#define API_CTRL_CAM_EN         (API_CTRL_CAM_OPEN|API_CTRL_CAM_STAR)
#define API_CTRL_CAM_DIS        (API_CTRL_CAM_CLOS|API_CTRL_CAM_STOP)

//#define API_CTRL_CAM_DISP_EN    (API_CTRL_CAM_EN|API_CTRL_DISP_OPEN)
//#define API_CTRL_CAM_DISP_DIS   (API_CTRL_CAM_DIS|API_CTRL_DISP_CLOS)

enum DEV_INIT_STATE
{
    DEV_INIT_STATE_UNINIT = 0,
    DEV_INIT_STATE_INITED,
    DEV_INIT_STATE_ERROR,
};

enum kdp_device_status_code_e{
    KDP_DEVICE_STATUS_OK = 0,
    KDP_DEVICE_STATUS_ERROR = -1,
    KDP_DEVICE_STATUS_IDLE,
    KDP_DEVICE_STATUS_BUSY,

    KDP_DEVICE_CAMERA_NULL = 0x100,
    KDP_DEVICE_CAMERA_IDLE,
    KDP_DEVICE_CAMERA_RUNNING,
    KDP_DEVICE_CAMERA_IDLE_PERM,
    // KDP_DEVICE_CAMERA_INFERENCE,
    // KDP_DEVICE_CAMERA_REGISTRATION,

    // KDP_DEVICE_DISPLAY_CLOSED = 0x200,
    // KDP_DEVICE_DISPLAY_OPENED,
    // KDP_DEVICE_DISPLAY_RUNNING
};

enum CAM_THREAD_STATE
{
    CAM_THREAD_STATE_NULL = 0,
    CAM_THREAD_STATE_OPEN,
    CAM_THREAD_STATE_START,
    CAM_THREAD_STATE_STOP,
};

enum DEV_STATE
{
    DEV_STATE_NULL = 0,
    DEV_STATE_IDLE,
    DEV_STATE_RUN,
};

enum PERM_STATE
{
    PERMANENT_NULL = 0,
    PERMANENT_CAM,
    PERMANENT_DISP,
    PERMANENT_GUI,
    PERMANENT_CAM_DISP,
    PERMANENT_DISABLE,
};

enum CTRL_STATE
{
    CTRL_COMM = 0,  //For console
    CTRL_CMD,
    CTRL_GUI,
};

//extern BOOL g_bBootupGuiHighPriority;
//extern enum CAM_THREAD_STATE g_eCamThreadState;

s32 kl520_api_cam_disp_ctrl(u8 nCt, unsigned int nCamIdx, enum PERM_STATE ePermSt);
s32 kl520_api_cam_disp_close_perm_state_chk(void);
s32 kl520_api_cam_disp_state_rst(void);
s32 kl520_api_disp_open_chk(void);
s32 kl520_api_disp_close_chk(void);
void kl520_api_disp_resolution_set(u32 nW, u32 nH);

extern void kl520_api_hmi_ctrl_state_reset(enum CTRL_STATE ePermSt);
extern void kl520_api_hmi_ctrl_state_set(enum CTRL_STATE ePermSt);
enum kdp_device_status_code_e kl520_api_cam_state_get(unsigned int nCamIdx);
extern BOOL kl520_api_cam_to_dp_thread_alive(void);

extern u16 kl520_api_set_exposure_only( u8 nCamIdx, u32 nExpTime );

#endif
