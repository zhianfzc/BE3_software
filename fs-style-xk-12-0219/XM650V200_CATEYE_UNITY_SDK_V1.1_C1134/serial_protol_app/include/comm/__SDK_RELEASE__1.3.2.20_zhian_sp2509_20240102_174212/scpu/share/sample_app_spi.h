#ifndef __SAMPLE_APP_SPI_H__
#define __SAMPLE_APP_SPI_H__

#include "types.h"
#include "cmsis_os2.h"
#include "board_cfg.h"
#include "kl520_api_ssp.h"



/*下行数据，由telink发送给520*/
typedef enum _FDR_DN {
    FDN_CHECK      =   0x50,        //鉴权命令
    FDN_RECORD     =   0x51,        //人脸录入命令
    FDN_DETECT     =   0x52,        //人脸识别命令    人脸识别的成功与失败状态
    FDN_DELETE     =   0x53,        //删除faceid命令
    FDN_SHUTDOWN   =   0x54,        //关机通知指令
    FDN_SECRET     =   0x55,        //密码校验响应(下行)
    FDN_VERSION    =   0x56,        //获取版本号
    FDN_EVENT      =   0x57,        //主控事件上报
    FDN_OTA        =   0x58,        //进入OTA模式
    FDN_FACTORY    =   0x59,        //进入恢复出厂设置模式
    FDN_FAIL_CNT   =   0x5A,        //开锁失败次数响应
    FDN_AMT        =   0x5B,        //通知进入AMT模式
    FDN_SETTING    =   0x5C,        //通知进入设置模式
    FDN_ACTION     =   0x5D,
    FDN_CHAR       =   0x5E,
    FDN_ACTREL     =   0x5F,      	//指纹识别的成功与失败状态；
    FDN_KEYBD      =   0x60,	   		//通知进入键盘模式；
    FDN_FP_ADD     =   0x61,	   	 	//本地模式通知进入指纹添加；
    FDN_SYSLOCK    =   0x62,        //系统锁定通知
    FDN_LOW_PW     =   0x63,				//低电量提示
    FDN_MULTI_DELETE =   0x64,	  	//除了某个faceid，其他全删
    FDN_RECORD_RET	 =	 0x65,		  //人脸录入命令结果
    FDN_DETECT_RET	 =	 0x66,		  //人脸识别主动上报
    FDN_DETECT_NOTIFY = 0x67,		 		//主控识别结果通知52
    FDN_POWON = 0x68,								//开机动画
    FDN_POWON_END = 0x69,
    FDN_CHAR_REC = 0x6A,
    FDN_EXIT_AMT = 0x6B,
    FDN_FP_TIMEOUT = 0x6C,
    FDN_VOICE_PLAY = 0x6D,//520通知主控需要播放哪些语音
    FDN_CANCEL_NOTIFY  = 0x6E,
    FDN_REC_NEXT_USR = 0x6F,
    FDN_ENTER_SKB = 0x70,
    FDN_AMT_VERIFY  = 0x71,
    FDN_AMT_LED = 0x72,
    FDN_LANGUAGE = 0x73,
    FDN_USR_FULL = 0x74,
    FDN_TIMESTAMP = 0x75,
    FDN_AMT_VERIFY_SUCCESS = 0x76,
    FDN_FACE_SNAP_FUNC_SET = 0x77,
    FDN_ADMIN_FACE_ID_NOTIFY = 0x78, //admin id
    FDN_VIEW_UNLOCK_FACES_NOTIFY = 0x79,//解锁相册
    FDN_KEY_LEN = 0x7a,//密码长度
    FDN_MAX
}FDR_DN;


/*上行数据，由520发送给telink*/
typedef enum _FDR_UP {
    FUP_CHECK      =   0xA0,       //鉴权响应
    FUP_RECORD     =   0xA1,       //人脸录入响应
    FUP_DETECT     =   0xA2,       //人脸识别响应
    FUP_DELETE     =   0xA3,       //删除faceid响应
    FUP_SHUTDOWN   =   0xA4,       //关机通知响应
    FUP_SECRET     =   0xA5,       //密码校验请求(上行)
    FUP_VERSION    =   0xA6,       //520软件版本号响应
    FUP_EVENT      =   0xA7,       //520对事件消息的响应
    FUP_OTA        =   0xA8,       //520对OTA命令的响应
    FUP_FACTORY    =   0xA9,       //恢复出厂设置命令的响应
    FUP_FAIL_CNT   =   0xAA,       //开锁失败次数查询请求
    FUP_AMT        =   0xAB,       //AMT测试结果响应
    FUP_SETTING    =   0xAC,       //设置模式响应
    FUP_ACTION     =   0xAD,       //520动作上报 重试 延长时间
    FUP_CHAR       =   0xAE,       //520按键字符上报
    FUP_ACTREL     =   0xAF,
    FUP_KEYBD      =   0xB0,
    FUP_FP_ADD     =   0xB1,
    FUN_SYSLOCK    =   0xB2,
    FUN_LOW_PW     =   0xB3,
    FUN_MULTI_DELETE =   0xB4,	  //除了某个faceid，其他全删
	FUN_RECORD_RET	 =	 0xB5,		  //人脸录入命令结果
	FUN_DETECT_RET	 =	 0xB6,		  //人脸识别主动上报
    FUN_DETECT_NOTIFY =  0xB7,		 //主控识别结果通知520
    FUN_POWON = 0xB8,
    FUN_POWON_END = 0xB9,
    FUN_CHAR_REC = 0xBA,
    FUN_EXIT_AMT = 0xBB,
    FUN_FP_TIMEOUT = 0xBC,
    FUN_VOICE_PLAY = 0xBD,
    FUN_CANCEL_NOTIFY  = 0xBE,
    FUN_REC_NEXT_USR = 0xBF,
    FUN_ENTER_SKB = 0xC0,
    FUN_AMT_VERIFY  = 0xC1,
    FUN_AMT_LED = 0xC2,
    FUN_LANGUAGE = 0xC3,
    FUN_USR_FULL = 0xC4,
    FUN_TIMESTAMP = 0xC5,
    FUN_AMT_VERIFY_SUCCESS = 0xC6,
    FUN_FACE_SNAP_FUNC_SET = 0xC7,
    FUN_ADMIN_FACE_ID_NOTIFY = 0xC8,
    FUN_VIEW_UNLOCK_FACES_NOTIFY = 0xC9,
    FUN_KEY_LEN = 0xCA,
    FUP_MAX
}FDR_UP;






#endif
