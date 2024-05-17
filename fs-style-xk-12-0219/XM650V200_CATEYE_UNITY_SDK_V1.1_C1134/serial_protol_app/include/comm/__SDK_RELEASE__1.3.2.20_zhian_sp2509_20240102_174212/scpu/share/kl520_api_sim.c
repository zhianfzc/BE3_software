#include "kl520_api_sim.h"
#include "kl520_include.h"
#include "kl520_api.h"
#include "kl520_api_snapshot.h"
#include "kl520_api_fdfr.h"
#include "sample_app_console.h"
#include "kl520_api_fdfr.h"

u16 m_fdfr_counter = 0;
BOOL kl520_api_sim_is_running(void)
{
    return ((kdp_e2e_prop_get2(flow_mode) > FLOW_MODE_SIM_VIRTUAL));
    //return snapshot_fdfr_sim;
}

void kl520_api_sim_set_rst(void)
{
    m_fdfr_counter=0;
}

#if CFG_USB_SIMTOOL == 1

//bool snapshot_fdfr_sim = FALSE;
u8 m_user_cmd = 0;
u16 m_sim_counter = 0;

osThreadId_t tid_sim_fdfr = NULL;

void _tasks_sim_fdfr_thread(void *arg)
{
    kl520_sim_ctx *ctx = (kl520_sim_ctx*)arg;
#if CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_fdfr_catch(MIPI_CAM_RGB);
#endif
    //kl520_api_fdfr_set_flow_mode(TRUE);
    ctx->result = kl520_api_fdfr_element();
    //kl520_api_fdfr_set_flow_mode(FALSE);

//    kl520_api_snapshot_addr(MIPI_CAM_INF);
    kl520_api_sim_write_json();

    kdp_e2e_prop_set2(flow_mode, FLOW_MODE_NORMAL);
    //snapshot_fdfr_sim = FALSE;
    dbg_msg_user("fdfr_sim_only finish...");
    tid_sim_fdfr = NULL;
    osThreadExit();
}

int kl520_api_sim_state(void)
{
    return ((int)tid_sim_fdfr);
}

void kl520_api_sim_set_usercmd(u8 id)
{
    m_user_cmd = id;
}

u8 kl520_api_sim_get_usercmd(void)
{
    return m_user_cmd;
}


void kl520_api_sim_set_fdfr(void)
{
    m_fdfr_counter++;
}

u8 kl520_api_sim_get_fdfr(void)
{
    return m_fdfr_counter;
}



u16 kl520_api_sim_get_count(void)
{
    return ((kl520_api_sim_is_running()) ? m_sim_counter : 0);
}


void kl520_api_sim_fdfr(kl520_sim_ctx *ctx)
{
    if(tid_sim_fdfr == NULL)
    {
        kl520_api_fdfr_terminate_thread();
        kdp_e2e_reset();
        m_face_mode = FACE_MODE_RECOGNITION_TEST;
        kdp_e2e_prop_set2(face_mode, FACE_MODE_RECOGNITION_TEST);

        kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_MODELS);
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
        kl520_api_snapshot_inf_decoder();
#endif

        //snapshot_fdfr_sim = TRUE;
        m_sim_counter++;


        osThreadAttr_t attr = {
            .stack_size = 1024,
            .attr_bits = osThreadJoinable
        };


        tid_sim_fdfr = osThreadNew(_tasks_sim_fdfr_thread, (void*)ctx, &attr);
        dbg_msg_user("fdfr_sim_only start..., %d",osThreadGetCount());
        osThreadJoin(tid_sim_fdfr);
        kdp_e2e_prop_set2(face_mode, FACE_MODE_NONE);
        m_face_mode = FACE_MODE_NONE;
    }
}

void kl520_api_sim_fdfr_flow_switch(void)
{
    kdp_e2e_prop *prop = kdp_e2e_prop_get_inst();
    if (FLOW_MODE_NORMAL == kdp_e2e_prop_get(prop, flow_mode)) {
        dbg_msg_api("[Snapshot]fdfr sim mode");
        kdp_e2e_prop_set(prop, flow_mode, FLOW_MODE_SIM_MODELS);
    }
    else {
        dbg_msg_api("[Snapshot]fdfr normal mode");
        kdp_e2e_prop_set(prop, flow_mode, FLOW_MODE_NORMAL);
    }
    // snapshot_fdfr_sim = !snapshot_fdfr_sim;

    // if(snapshot_fdfr_sim == TRUE){
    //     dbg_msg_api("[Snapshot]fdfr sim mode");
    // }
    // else{
    //     dbg_msg_api("[Snapshot]fdfr normal mode");
    // }
}

void kl520_api_sim_face_add(kl520_sim_ctx *ctx, u32 face_add_type, u32 face_mode)
{
    int ret = 0;
    if(FACE_ADD_TYPE_NORMAL == face_add_type)
    {
        kl520_api_face_close();
    }

    kl520_api_dp_five_face_enable();

    if(0 == face_mode)
        kl520_api_face_set_add_mode(FACE_ADD_MODE_5_FACES);
    else{
        kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
    }

    kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_MODELS);
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_inf_decoder();
#endif

#if ( KDP_BIT_CTRL_MODE == YES )
    kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_bit_ctrl_face_add)kl520_api_face_idx_chg_2_bit_ctrl((u8)face_add_type));
#else
    kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_face_add_type)face_add_type);
#endif
    ret = kl520_api_add_wait_and_get();

    kdp_e2e_face_variables *face_var = kdp_e2e_get_face_variables();

    ctx->result = (s32)( (face_var->rgb_age_group  << 16) | (m_curr_face_id << 8) | ret);
    
    kl520_api_sim_write_json();

    if((FACE_ADD_TYPE_DOWN <= face_add_type) || (ret != E2E_OK)
        || kl520_api_face_get_add_mode() == FACE_ADD_MODE_1_FACE)
    {
        kl520_api_face_close();
    }
}

void kl520_api_sim_face_pre_add(kl520_sim_ctx *ctx, u32 face_add_type)
{

    kdp_e2e_face_variables *face_var = kdp_e2e_get_face_variables();

    int ret = 0;

    if(FACE_ADD_TYPE_NORMAL == face_add_type)
    {
        kl520_api_face_close();
    }

    kl520_api_face_set_add_mode(FACE_ADD_MODE_1_FACE);
    kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_PRE_ADD);
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_inf_decoder();
#endif

    face_var->pre_add = AI_TYPE_PR1;

#if ( KDP_BIT_CTRL_MODE == YES )
    kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_bit_ctrl_face_add)kl520_api_face_idx_chg_2_bit_ctrl((u8)face_add_type));
#else
    kl520_api_face_add(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, (kl520_face_add_type)face_add_type);
#endif

    ret = kl520_api_add_wait_and_get();

    ctx->result = (s32)( (face_var->rgb_age_group  << 16) | (m_curr_face_id << 8) | ret);

    face_var->pre_add = 0;

    //if((FACE_ADD_TYPE_DOWN <= face_add_type) || (ret != E2E_OK))
    {
        kl520_api_face_close();
    }

    kdp_e2e_prop_set2(flow_mode, FLOW_MODE_NORMAL);

}

void kl520_api_sim_face_recognition(kl520_sim_ctx *ctx)
{
    int ret = KL520_APP_FLAG_FDFR_ERR;
    u8 face_id = 0xFF;

    kdp_e2e_prop_set2(flow_mode, FLOW_MODE_SIM_MODELS);

    sample_force_abort_enable();
#if CFG_SNAPSHOT_ENABLE == 1 || CFG_SNAPSHOT_ENABLE == 2
    kl520_api_snapshot_inf_decoder();
#endif

    kl520_api_face_recognition(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    u32 events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
    if(events != KL520_DEVICE_FLAG_ERR)
    {
        kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
        ret = kl520_api_face_get_result(&face_id);
        if (KL520_FACE_OK == ret) {
            dbg_msg_console("[%s] KL520_FACE_OK, face_id=0x%x, UserName=%s", __func__, face_id, vars_cur->user_name);
        }
        else if (KL520_FACE_DB_FAIL == ret) {
            dbg_msg_console("[%s] KL520_FACE_DB_FAIL, face_id=0x%x, UserName=%s", __func__, face_id, vars_cur->user_name);
        }
        else {
            dbg_msg_console("[%s] ERROR, ret=0x%x", __func__, ret);
        }
    }

    if(tid_abort_thread != 0)
        sample_force_abort_disable();

    ctx->result = (s32)((face_id << 8) | ret);
    kl520_api_sim_write_json();

    kl520_api_face_close();
}

#if CFG_COMPARE_1VS1 == YES

void kl520_api_sim_2user_comapre(kl520_sim_ctx *ctx)
{
    int ret = KL520_APP_FLAG_FDFR_ERR;
    u8 similarity = 0xFF;

    dbg_msg_console("[%s] sim = %d", __func__, ctx->result);


    kdp_e2e_prop_set2(flow_mode, (kdp_e2e_flow_mode)ctx->result );

    sample_force_abort_enable();

    kl520_api_face_2user_compare(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    u32 events = wait_event(kl520_api_get_event(), KL520_APP_FLAG_FDFR | KL520_DEVICE_FLAG_ERR);
    if(events != KL520_DEVICE_FLAG_ERR)
    {
        kdp_e2e_face_variables *vars_cur = kdp_e2e_get_face_variables();
        ret = kl520_api_face_get_result(&similarity);

        if (KL520_FACE_OK == ret) {
            dbg_msg_console("[%s] KL520_FACE_OK, s= %d", __func__, similarity );
        }
        else if (KL520_FACE_SEND_NEXT_IMAGE == ret) {
            dbg_msg_console("[%s] KL520_FACE_SEND_NEXT_IMAGE", __func__ );
        }
        else {
            dbg_msg_console("[%s] ERROR, ret=0x%x, s= %d", __func__, ret, similarity);
        }
    }

    if(tid_abort_thread != 0)
        sample_force_abort_disable();

    ctx->result = (s32)( (similarity << 8) | ret);

    kl520_api_face_close();
}
#endif // CFG_COMPARE_1VS1

#endif
