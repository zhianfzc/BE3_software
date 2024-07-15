#ifndef __ISP_ERRORS_H__
#define __ISP_ERRORS_H__


#define  ISP_ERR_INVALID_DEVID      (0x001)  //设备号无效  
#define  ISP_ERR_INVALID_CHNID      (0x002)  //通道号无效  
#define  ISP_ERR_INVALID_PARA       (0x003)  //参数设置无效  
#define  ISP_ERR_INVALID_NULL_PTR   (0x006)  //输入参数空指针错误  
#define  ISP_ERR_FAILED_NOTCONFIG   (0x007)  //设备或通道属性未配置  
#define  ISP_ERR_NOT_SUPPORT        (0x008)  //操作不支持  
#define  ISP_ERR_NOT_PERM           (0x009)  //操作不允许  
#define  ISP_ERR_NOMEM              (0x00C)  //分配内存失败  
#define  ISP_ERR_BUF_EMPTY          (0x00E)  //输入缓存为空  
#define  ISP_ERR_BUF_FULL           (0x00F)  //输入缓存为满  
#define  ISP_ERR_SYS_NOTREADY       (0x010)  //输入系统未初始化   
#define  ISP_ERR_BUSY               (0x012)  //输入系统忙  
#define  ISP_ERR_FAILED_NOTENABLE   (0x040)  //输入设备或通道未启用  
#define  ISP_ERR_FAILED_NOTDISABL   (0x041)  //输入设备或通道未禁用
#define  ISP_ERR_FAILED_CHNOTDISA   (0x042)  //通道未禁用
#define  ISP_ERR_CFG_TIMEOUT        (0x043)  //配置属性超时  
#define  ISP_ERR_NORM_UNMATCH       (0x044)  //不匹配  
#define  ISP_ERR_INVALID_WAYID      (0x045)  //视频通路号无效  
#define  ISP_ERR_INVALID_PHYCHNID   (0x046)  //视频物理通道号无效  
#define  ISP_ERR_FAILED_NOTBIND     (0x047)  //视频通道未绑定  
#define  ISP_ERR_FAILED_BINDED      (0x048)  //视频通道已绑定  
#define  ISP_ERR_DIS_PROCESS_FAIL   (0x049)  //DIS 运行失败  


#define ISP_ERR_PHYVI				(0x10000000)


#endif

