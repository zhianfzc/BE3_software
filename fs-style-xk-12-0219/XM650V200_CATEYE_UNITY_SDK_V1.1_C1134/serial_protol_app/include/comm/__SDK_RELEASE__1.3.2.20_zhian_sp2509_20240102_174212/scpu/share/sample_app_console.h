#ifndef __SAMPLE_APP_CONSOLE_H__
#define __SAMPLE_APP_CONSOLE_H__
#include "board_kl520.h"
#include "kl520_com.h"

#include "kl520_api_camera.h"

extern osThreadId_t tid_abort_thread;

void sample_doorlock_entry(void);

extern uint16_t uart_sample_face_add_timeout(uint16_t time_out_ms);
extern uint16_t uart_sample_face_recognition_timeout(uint16_t time_out_ms);
extern uint16_t uart_sample_face_mp_timeout(uint16_t time_out_ms);
extern uint16_t uart_sample_face_del_all(void);
extern uint16_t uart_sample_face_del_user(uint8_t face_id);
extern void sample_face_close(void);

extern void sample_factory(void);
extern void sample_switch_rbg_nir(void);
extern void sample_open_video_renderer(void);
extern void sample_close_video_renderer(void);

extern void sample_force_abort_enable(void);
extern void sample_force_abort_disable(void);

void sample_snapshot_auto_usb_mode(u8 adv_mode);
void switch_scpu_part(void);

#endif
