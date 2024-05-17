#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "isp_type.h"
#include "isp_i2c.h"
#include "isp_print.h"
#include "XAx_cmos.h"
#include "XAx_sensor_ctl.h"

extern int i2c_write(unsigned char i2cnum, unsigned char devaddr, unsigned int regaddr, unsigned int reglen, unsigned int data, unsigned int datalen);
extern int i2c_read(unsigned char i2cnum, unsigned char devaddr, unsigned int regaddr, unsigned int reglen, unsigned int datalen);

ISP_S32 sensor_read_register(ISP_U8 Ch, ISP_U32 addr)
{
	ISP_S32 ret;
	I2C_DATA_S i2c_data;
	i2c_data.dev_addr = gstSnsAttr[Ch].stSnsI2C.u8DevAddr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = gstSnsAttr[Ch].stSnsI2C.u32RegAddrByte;
	i2c_data.data_byte_num = gstSnsAttr[Ch].stSnsI2C.u32RegDataByte;
	ret = i2c_read(gstSnsAttr[Ch].stSnsI2C.u8Channel, i2c_data.dev_addr, i2c_data.reg_addr, i2c_data.addr_byte_num, i2c_data.data_byte_num);
	return (ret==-1)?-1:ret;
}


ISP_S32 sensor_write_register(ISP_U8 Ch, ISP_U32 addr, ISP_U32 data)
{
	ISP_S32 ret;
	I2C_DATA_S i2c_data;
	i2c_data.dev_addr = gstSnsAttr[Ch].stSnsI2C.u8DevAddr;
	i2c_data.reg_addr = addr;
	i2c_data.addr_byte_num = gstSnsAttr[Ch].stSnsI2C.u32RegAddrByte;
	i2c_data.data = data;
	i2c_data.data_byte_num = gstSnsAttr[Ch].stSnsI2C.u32RegDataByte;
	ret = i2c_write(gstSnsAttr[Ch].stSnsI2C.u8Channel, i2c_data.dev_addr, i2c_data.reg_addr, i2c_data.addr_byte_num, i2c_data.data, i2c_data.data_byte_num);
	return ret?-1:0;
}


void sensor_init(ISP_U8 u8IspCh)
{
	if(gstSnsAttr[u8IspCh].pfn_cmos_sensor_init)
	{
		(gstSnsAttr[u8IspCh].pfn_cmos_sensor_init)(u8IspCh);
	}
	return;
}





