#ifndef __KDP520_I2C_H__
#define __KDP520_I2C_H__


#include "framework/framework_driver.h"

#define I2C_INTERRUPT_ENABLE    0

#define I2C_NAME_SIZE       16
#define I2C_MASTER_WRITE    0x00 /* write data, from master to slave */
#define I2C_MASTER_READ     0x01 /* read data, from slave to master */

enum i2c_adap_id {
    I2C_ADAP_0 = 0,
    I2C_ADAP_1    ,
    I2C_ADAP_2    ,
    I2C_ADAP_3    ,
    I2C_ADAP_MAX  ,
};

struct i2c_msg {
    u16 addr;   /* slave address */
    u8 flags;
    u8 len;     /* msg length */
    u8 *buf;    /* pointer to msg data */
};

/* public api */
int kdp_drv_i2c_init(enum i2c_adap_id id, unsigned long bus_speed, BOOL force);
int kdp_drv_i2c_write(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 data);
int kdp_drv_i2c_read(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 *lpdata);
int kdp_drv_i2c_read_bytes(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 *lpdata, u8 data_len);
    
#endif 
