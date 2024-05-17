/**
 * @file      dbg.h
 * @brief     debug macro 
 * @copyright (c) 2018 Kneron Inc. All right reserved.
 */

#ifndef __DBG_H__
#define __DBG_H__

#include <stdio.h>
#include "ipc.h"

//#define DEV_TEST_VERSION
//#define DEV_PKT_LOG_DETAIL

#define LOG_NONE        0
#define LOG_USER        1
#define LOG_CRITICAL    1
#define LOG_ERROR       2

#define LOG_INFO        4
#define LOG_TRACE       5
#define LOG_DBG         6
#define LOG_PROFILE     9

#define DEBUG_CONSOLE                  DRVUART_PORT0

#ifdef DEV_TEST_VERSION
#undef CUSTOMER_SETTING_REMOVE_LOG
#else
#define CUSTOMER_SETTING_REMOVE_LOG
#endif

#ifdef  LOG_ENABLE

#ifdef TARGET_NCPU

extern void fLib_printf(const char *f, ...);

extern struct scpu_to_ncpu_s *in_comm_p;
extern int ncpu_debug_level;
#define log_get_level_ncpu()    (in_comm_p->debug_flags & 0x0000000F)
    
#ifdef CUSTOMER_SETTING_REMOVE_LOG
    #define dbg_msg(fmt, ...) 
    #define trace_msg(fmt, ...) 
    #define info_msg(fmt, ...) 
    #define err_msg(fmt, ...) fLib_printf(fmt, ##__VA_ARGS__)
    #define critical_msg(fmt, ...) 
    #define profile_msg(fmt, ...) 
    #define dbg_msg_algo(fmt, ...) //MSG(LOG_CRITICAL, fmt, ##__VA_ARGS__) 
    #define dbg_msg_algo2(fmt, ...) //MSG(LOG_CRITICAL, fmt, ##__VA_ARGS__) 
#else
    #define dbg_msg(fmt, ...) //MSG(LOG_DBG, fmt, ##__VA_ARGS__)
    #define trace_msg(fmt, ...) //MSG(LOG_TRACE, fmt, ##__VA_ARGS__)
    #define info_msg(fmt, ...) //MSG(LOG_INFO, fmt, ##__VA_ARGS__)
    #define err_msg(fmt, ...) fLib_printf(fmt, ##__VA_ARGS__)
    #define critical_msg(fmt, ...) fLib_printf(fmt, ##__VA_ARGS__)
    #define profile_msg(fmt, ...) fLib_printf(fmt, ##__VA_ARGS__)
    #define dbg_msg_algo(fmt, ...) fLib_printf(fmt, ##__VA_ARGS__)
    #define dbg_msg_algo2(fmt, ...) //MSG(LOG_CRITICAL, fmt, ##__VA_ARGS__) 
#endif   
    
    
#else // TARGET_SCPU

extern struct scpu_to_ncpu_s *s_out_comm;
extern int scpu_debug_level;

#define MSG(level, format, ...) \
    do {                                                   \
        if (level <= scpu_debug_level)                     \
            kdp_printf(format, ##__VA_ARGS__);            \
    } while (0)


void kdp_printf(const char *f, ...);
void kdp_level_printf(int level, const char *fmt, ...);
void kdp_user_level_printf(int level, const char *fmt, ...);
void kdp_printf_nocrlf(const char *f, ...);
    
#define dbg_msg(fmt, ...) //MSG(LOG_DBG, fmt, ##__VA_ARGS__)
#define trace_msg(fmt, ...) MSG(LOG_TRACE, fmt, ##__VA_ARGS__)
#define info_msg(fmt, ...) //MSG(LOG_INFO, fmt, ##__VA_ARGS__)
#define err_msg(fmt, ...) kdp_level_printf(LOG_ERROR, fmt, ##__VA_ARGS__)
#define critical_msg(fmt, ...) MSG(LOG_CRITICAL, fmt, ##__VA_ARGS__)
#define profile_msg(fmt, ...) MSG(LOG_PROFILE, fmt, ##__VA_ARGS__)


#ifdef CUSTOMER_SETTING_REMOVE_LOG
    #define dbg_msg_console(__format__, ...) kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_err(__format__, ...) kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_flash(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_camera(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_display(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_touch(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_com(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_gui(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_app(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_e2e(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_api(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_usb(__format__, ...) //{ kdp_printf(__format__"\r\n", ##__VA_ARGS__); }
    #define dbg_msg_algo(__format__, ...) //kdp_level_printf(LOG_CRITICAL, __format__"\r\n", ##__VA_ARGS__)
    #define dlog(__format__, ...) //kdp_level_printf(LOG_DBG, "[%s][%s] " __format__ "\r\n", DEF_LOG_CATEG, __func__, ##__VA_ARGS__)
    #define dbg_msg_model(__format__, ...) kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_ncpu(__format__, ...) //kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_engineering(__format__, ...) //kdp_user_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_tile(__format__, ...) //kdp_level_printf(LOG_USER, __format__, ##__VA_ARGS__)
#else
    #define dbg_msg_console(__format__, ...) kdp_printf(__format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_err(__format__, ...) kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_flash(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_camera(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_display(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_touch(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_com(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_gui(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_app(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_e2e(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_api(__format__, ...) //kdp_level_printf(LOG_ERROR, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_usb(__format__, ...) { kdp_printf(__format__"\r\n", ##__VA_ARGS__); }
    #define dbg_msg_algo(__format__, ...) kdp_level_printf(LOG_CRITICAL, __format__"\r\n", ##__VA_ARGS__)
    #define dlog(__format__, ...) kdp_level_printf(LOG_DBG, "[%s][%s] " __format__ "\r\n", DEF_LOG_CATEG, __func__, ##__VA_ARGS__)
    #define dbg_msg_model(__format__, ...) kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)    
    #define dbg_msg_ncpu(__format__, ...) //kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_engineering(__format__, ...) //kdp_user_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
    #define dbg_msg_tile(__format__, ...) kdp_level_printf(LOG_USER, __format__, ##__VA_ARGS__)
#endif 

#define dbg_msg_user(__format__, ...) kdp_user_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)
#define dbg_msg_nocrlf(__format__, ...) { kdp_printf_nocrlf(__format__, ##__VA_ARGS__); }
#define dbg_msg_console_zhian(__format__, ...) kdp_level_printf(LOG_USER, __format__"\r\n", ##__VA_ARGS__)


#endif

#else

#endif // LOG_ENABLE

uint32_t log_get_level_scpu(void);
uint32_t log_get_user_level_scpu(void);
void log_set_level_scpu(uint32_t level);
void log_set_level_ncpu(uint32_t level);
void log_set_user_level_scpu(uint32_t level);
void log_set_user_level_ncpu(uint32_t level);

#define kmdw_console_set_log_level_scpu log_set_level_scpu
#define kmdw_console_set_log_level_ncpu log_set_level_ncpu

#define ASSERT(x)   do { \
                        if (!(x)) 	\
                            for (;;)	\
                                ; 		\
                    } while (0)

#endif // __DBG_H__
