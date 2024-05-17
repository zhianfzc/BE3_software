#include "board_kl520.h"
#include "kdp520_i2c.h"
#include <string.h> 
#include <stdlib.h>
#include "kneron_mozart.h"
#include "scu_reg.h"
#include "types.h"
#include "base.h"
#include "io.h"
#include "delay.h"
#include "framework/ioport.h"
#include "framework/mutex.h"
#include "framework/framework_driver.h"
#include "framework/framework_errno.h"
#include "media/camera/sensor.h"
#include "dbg.h"


#define I2C_GSR_Value       5
#define I2C_TSR_Value       5
#define I2C_MAX_FREQ        (400 * 1000)//(400 * 1024)

#define I2C_RD(dev)         ((((dev) << 1) & 0xfe) | 1)
#define I2C_WR(dev)         (((dev) << 1) & 0xfe)


/* I2C Register */
#define I2C_REG_CONTROL     0x0
#define I2C_REG_STATUS      0x4
#define I2C_REG_CLOCKDIV    0x8
#define I2C_REG_DATA        0xC
#define I2C_REG_TGSR        0x14

/* I2C control register */
#define I2C_CTRL_ALIRQ      0x2000  /* arbitration lost interrupt (master) */
#define I2C_CTRL_NAKRIRQ    0x400   /* NACK response interrupt (master) */
#define I2C_CTRL_DRIRQ      0x200   /* rx interrupt (both) */
#define I2C_CTRL_DTIRQ      0x100   /* tx interrupt (both) */
#define I2C_CTRL_TBEN       0x80    /* tx enable (both) */
#define I2C_CTRL_NAK        0x40    /* NACK (both) */
#define I2C_CTRL_STOP       0x20    /* stop (master) */
#define I2C_CTRL_START      0x10    /* start (master) */
#define I2C_CTRL_SCLEN      0x4     /* enable clock out (master) */
#define I2C_CTRL_I2CEN      0x2     /* enable I2C (both) */
#define I2C_CTRL_ENABLE     \
    (I2C_CTRL_SCLEN | I2C_CTRL_I2CEN)
#define I2C_CTRL_ENABLE_IRQ     \
    (I2C_CTRL_ALIRQ | I2C_CTRL_NAKRIRQ | I2C_CTRL_DRIRQ | I2C_CTRL_DTIRQ)

/* I2C Status register */
#define I2C_STATUS_TD       0x20    /* tx/rx finish */
#define I2C_STATUS_BUSY     0x4     /* chip busy */	

#define I2C_WAIT_INT_TIMEOUT_CNT    (0x001F)//0x7FFFF)

#if CFG_I2C_0_ENABLE == YES
extern struct core_device kdp520_i2c_0;
#endif
#if CFG_I2C_1_ENABLE == YES
extern struct core_device kdp520_i2c_1;
#endif
#if CFG_I2C_2_ENABLE == YES
extern struct core_device kdp520_i2c_2;
#endif
#if CFG_I2C_3_ENABLE == YES
extern struct core_device kdp520_i2c_3;
#endif

osMutexId_t mutex_i2c = NULL;

struct kdp520_i2c_context {
    u32 base;
    int retries;
    struct kdp520_i2c_params params;
    struct mutex lock;
#if I2C_INTERRUPT_ENABLE
    IRQn_Type irq;
    osEventFlagsId_t evt_isr_id;
#endif
    int (*master_xfer)(struct kdp520_i2c_context *i2c_ctx, struct i2c_msg *msgs, int num);    
};

struct kdp520_i2c_context i2c_ctx_s[I2C_ADAP_MAX];

u32 _kdp520_i2c_getgsr(struct kdp520_i2c_context *i2c_ctx)
{
    u32 tmp;

    tmp = inw(((u8*)i2c_ctx->base + I2C_REG_TGSR));

    return ((tmp >> 10) & 0x7);
}

static BOOL _kdp_i2c_check_busy(u32 base) {
    if((inw(base + I2C_REG_STATUS) & I2C_STATUS_BUSY)== I2C_STATUS_BUSY)
     return TRUE;

    return FALSE;
}

#if I2C_INTERRUPT_ENABLE
static int _kdp_i2c_wait(struct kdp520_i2c_context *ctx, uint32_t mask)
{
    int ret = -KDP_FRAMEWORK_ERRNO_IO;

    if (osKernelRunning == osKernelGetState())
    {
        uint32_t flag = osEventFlagsWait(ctx->evt_isr_id, mask, osFlagsWaitAny, 1000);
        if(osErrorTimeout == flag)
        {
            ret = flag;
            err_msg("[%s] timeout!", __func__);
        }
        else
        {
            if(flag | mask)
            {
                ret = 0;
            }
        }
    }
    else
    {
        uint32_t stat, ts;

        for (ts = 0; ts < 100000; ts++)
        {
            stat = inw(ctx->base + I2C_REG_STATUS);
            outw(ctx->base + I2C_REG_STATUS, stat);
            if ((stat & mask) == mask)
            {
                ret = 0;//it means ok
                break;
            }
        }
    }
    
    return ret;
}
#else
static int _kdp_i2c_wait(int base, uint32_t mask)
{
    int ret = -KDP_FRAMEWORK_ERRNO_IO;
    uint32_t stat, ts;

    //for (ts = 0; ts < 55000; ts++) {
    for (ts = 0; ts < 1000; ts++) { // normally ts ~=200
        stat = inw(base + I2C_REG_STATUS);
        outw(base + I2C_REG_STATUS, stat);
        if ((stat & mask) == mask) {
            ret = 0;//it means ok
            break;
        }
    }

    return ret;
}
#endif

static int _kdp520_i2c_xfer_msg(
        struct kdp520_i2c_context *i2c_ctx, struct i2c_msg *msg, BOOL is_stop)
{
    int ret, i;

    u32 base = i2c_ctx->base;    
    u16 slave_addr = msg->addr;    
    u16 flags = msg->flags;
    u16 len = msg->len;
    u8 *buf = msg->buf;
    
    BOOL is_read = (flags & I2C_MASTER_READ);

    //dbg_msg("kdp520_i2c_xfer_msg base=%x slave_addr=%x len=%d is_stop=%d", base, slave_addr, len, is_stop);
    if (is_read)
        outw(base + I2C_REG_DATA, I2C_RD(slave_addr));
    else
        outw(base + I2C_REG_DATA, I2C_WR(slave_addr));

#if I2C_INTERRUPT_ENABLE
    if (osKernelRunning == osKernelGetState())
    {
        outw(base + I2C_REG_CONTROL, I2C_CTRL_ENABLE | I2C_CTRL_DRIRQ | I2C_CTRL_TBEN | I2C_CTRL_START);
    }
    else
    {
        outw(base + I2C_REG_CONTROL, I2C_CTRL_ENABLE | I2C_CTRL_TBEN | I2C_CTRL_START);
    }
    
    ret = _kdp_i2c_wait(i2c_ctx, I2C_STATUS_TD);
#else
    outw(base + I2C_REG_CONTROL, I2C_CTRL_ENABLE | I2C_CTRL_TBEN | I2C_CTRL_START);
    ret = _kdp_i2c_wait(base, I2C_STATUS_TD);
#endif
    if (0 == ret)
    {
#if I2C_INTERRUPT_ENABLE
        u32 ctrl = I2C_CTRL_ENABLE | I2C_CTRL_DRIRQ | I2C_CTRL_TBEN;
#else
        u32 ctrl = I2C_CTRL_ENABLE | I2C_CTRL_TBEN;
#endif
        for (i = 0; i < len; ++i) {
            if ((is_stop) && (i == len - 1)) {
                if (is_read)
                    ctrl |= I2C_CTRL_NAK | I2C_CTRL_STOP;
                else
                    ctrl |= I2C_CTRL_STOP;
            }
                
            if (!is_read)
                outw(base + I2C_REG_DATA, buf[i]);
            
            outw(base + I2C_REG_CONTROL, ctrl);
            
#if I2C_INTERRUPT_ENABLE
            ret = _kdp_i2c_wait(i2c_ctx, I2C_STATUS_TD);
#else
            ret = _kdp_i2c_wait(base, I2C_STATUS_TD);
#endif
            if (ret)
                break;
            
            if (is_read)
                buf[i] = (u8)(inw(base + I2C_REG_DATA) & 0xFF);
        }
    }

    //kdp520_i2c_reset(i2c_ctx);

    return ret;
}

static int _kdp520_i2c_xfer(
        struct kdp520_i2c_context *i2c_ctx, struct i2c_msg *msgs, int num)
{
    int ret = 0;
    int i;
    u32 cnt = 0;
    //enable i2c clock
    
    for (i = 0; i < num; ++i) {
        ret = _kdp520_i2c_xfer_msg(i2c_ctx, &msgs[i], (i == (num - 1)));
        if (0 != ret)
            break;
    }

    while (_kdp_i2c_check_busy(i2c_ctx->base)){
        delay_us(1);
        if (I2C_WAIT_INT_TIMEOUT_CNT < cnt)
        {
            dbg_msg("_kdp_i2c_check_busy time out :%d", cnt);
            break;
        }
        cnt++;
    }

    //disable i2c clock

    return ret;
}

static int _i2c_transfer(struct kdp520_i2c_context *i2c_ctx, struct i2c_msg *msgs, int num)
{
    int ret = -KDP_FRAMEWORK_ERRNO_IO;
    int try;

    mutex_lock(&i2c_ctx->lock);
    
    for (ret = 0, try = 0; try <= i2c_ctx->retries; try++) {
        ret = i2c_ctx->master_xfer(i2c_ctx, msgs, num);
        if (ret != -KDP_FRAMEWORK_ERRNO_AGAIN)
            break;
    }
    
    mutex_unlock(&i2c_ctx->lock);
    
    return ret;
}

#if I2C_INTERRUPT_ENABLE

static void i2c_isr(struct core_device *core_d)
{
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *i2c_ctx = &i2c_ctx_s[i2c_id];

    UINT32 IntrMaskedStat;

    IntrMaskedStat = inw(i2c_ctx->base + I2C_REG_STATUS);
    // clear CPIO interrupt
    outw(i2c_ctx->base + I2C_REG_STATUS, IntrMaskedStat);

    // set event
    set_event(i2c_ctx->evt_isr_id, IntrMaskedStat);
}

#if CFG_I2C_0_ENABLE == YES
void i2c0_isr(void)
{
    i2c_isr(&kdp520_i2c_0);
}
#endif

#if CFG_I2C_1_ENABLE == YES
void i2c1_isr(void)
{
    i2c_isr(&kdp520_i2c_1);
}
#endif

#if CFG_I2C_2_ENABLE == YES
void i2c2_isr(void)
{
    i2c_isr(&kdp520_i2c_2);
}
#endif

#if CFG_I2C_3_ENABLE == YES
void i2c3_isr(void)
{
    i2c_isr(&kdp520_i2c_3);
}
#endif

#endif	// #if I2C_INTERRUPT_ENABLE

static int _kdp520_i2c_probe(struct core_device *core_d)
{
    struct kdp520_i2c_context *i2c_ctx;

    int i2c_id = core_d->uuid;
    if (i2c_id >= I2C_ADAP_MAX) {
        err_msg("%s: err: i2c_id %d out of range\n", __func__, i2c_id);
        return -1;
    }

    i2c_ctx = &i2c_ctx_s[i2c_id];
    
    if(mutex_i2c == NULL)
        mutex_i2c = osMutexNew(NULL);
    
    switch (i2c_id) {
    case I2C_ADAP_3:
        i2c_ctx->base = IIC_FTIIC010_3_PA_BASE;
        break;
    case I2C_ADAP_2:
        i2c_ctx->base = IIC_FTIIC010_2_PA_BASE;
        break;
    case I2C_ADAP_1:
        i2c_ctx->base = IIC_FTIIC010_1_PA_BASE;
        break;
    case I2C_ADAP_0:
    default:
        i2c_ctx->base = IIC_FTIIC010_0_PA_BASE;
        break;
    }
#if I2C_INTERRUPT_ENABLE
        i2c_ctx->irq = IIC_FTIIC010_0_IRQ + i2c_id;
#endif

    i2c_ctx->retries = 5;
    i2c_ctx->master_xfer = _kdp520_i2c_xfer;

    mutex_create(&i2c_ctx->lock);

    return 0;
}

static int _kdp520_i2c_remove(struct core_device *core_d)
{
    return 0;
}

static int _kdp520_i2c_power(struct core_device *core_d, BOOL onoff)
{
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *i2c_ctx = &i2c_ctx_s[i2c_id];

    switch (i2c_ctx->base) {
        case IIC_FTIIC010_0_PA_BASE:
            masked_outw(SCU_REG_APBCLKG, 
                        ((onoff) ? (SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK) : (0)), 
                        SCU_REG_APBCLKG_PCLK_EN_I2C0_PCLK);
            break;
        case IIC_FTIIC010_1_PA_BASE:
            masked_outw(SCU_REG_APBCLKG, 
                        ((onoff) ? (SCU_REG_APBCLKG_PCLK_EN_I2C1_PCLK) : (0)), 
                        SCU_REG_APBCLKG_PCLK_EN_I2C1_PCLK);
            break;
        case IIC_FTIIC010_2_PA_BASE:
            masked_outw(SCU_REG_APBCLKG, 
                        ((onoff) ? (SCU_REG_APBCLKG_PCLK_EN_I2C2_PCLK) : (0)), 
                        SCU_REG_APBCLKG_PCLK_EN_I2C2_PCLK);
            break;
        case IIC_FTIIC010_3_PA_BASE:
            masked_outw(SCU_REG_APBCLKG, 
                        ((onoff) ? (SCU_REG_APBCLKG_PCLK_EN_I2C3_PCLK) : (0)), 
                        SCU_REG_APBCLKG_PCLK_EN_I2C3_PCLK);
            break;
        default:;
        
    }
    delay_us(500);

    return 0;
}

static int _kdp520_i2c_set_params(struct core_device *core_d, struct kdp520_i2c_params *params)
{
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    memcpy((void*)(&ctx->params), params, sizeof(ctx->params));
    
    return 0;
}

static int _kdp520_i2c_init(struct core_device *core_d)
{
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    u32 bus_speed = 0;

    outw(((u8*)ctx->base + I2C_REG_TGSR), (I2C_GSR_Value << 10) | I2C_TSR_Value);

    bus_speed = ctx->params.bus_speed;
    if (bus_speed > 0) {
        u32 bits = ((APB_CLOCK - (bus_speed * _kdp520_i2c_getgsr(ctx))) / ( 2 * bus_speed)) - 2;
        if (bits < BIT10) 
            outw(((u8*)ctx->base + I2C_REG_CLOCKDIV), bits);
        else {
            dbg_msg(" APB_CLOCK=%d, i_SCLout=%d",APB_CLOCK, bus_speed);
            dbg_msg("Pclk is to fast to form i2c clock, fail ");
        }
    }

#if I2C_INTERRUPT_ENABLE
    ctx->evt_isr_id = osEventFlagsNew(NULL);

    //register IRQ
    switch (ctx->irq)
    {
#if CFG_I2C_0_ENABLE == YES
        case IIC_FTIIC010_0_IRQ:
            NVIC_SetVector((IRQn_Type)IIC_FTIIC010_0_IRQ, (uint32_t)i2c0_isr);
            break;
#endif
#if CFG_I2C_1_ENABLE == YES
        case IIC_FTIIC010_1_IRQ:
            NVIC_SetVector((IRQn_Type)IIC_FTIIC010_1_IRQ, (uint32_t)i2c1_isr);
            break;
#endif
#if CFG_I2C_2_ENABLE == YES
        case IIC_FTIIC010_2_IRQ:
            NVIC_SetVector((IRQn_Type)IIC_FTIIC010_2_IRQ, (uint32_t)i2c2_isr);
            break;
#endif
#if CFG_I2C_3_ENABLE == YES
        case IIC_FTIIC010_3_IRQ:
            NVIC_SetVector((IRQn_Type)IIC_FTIIC010_3_IRQ, (uint32_t)i2c3_isr);
            break;
#endif
        default:
            break;
    }
    
    //enable IRQ
    if (osKernelRunning == osKernelGetState())
    {
        NVIC_EnableIRQ((IRQn_Type)(ctx->irq));
    }
    else
    {
        NVIC_DisableIRQ((IRQn_Type)(ctx->irq));
    }
#endif

    return 0;
}

static int _kdp520_i2c_reset(struct core_device *core_d)
{
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    u32 ts;
    outw((u8*)ctx->base + I2C_REG_CONTROL, 1);
    for (ts = 0; ts < 50000; ts++) {
        if (!(inw(((u8*)ctx->base + I2C_REG_CONTROL)) & 0x1)) {
            break;
        }
    }
    
    //dbg_msg("_kdp520_i2c_reset ts = %u", ts);

    return 0;    
}

static int _kdp520_i2c_write(
        struct core_device *core_d, u8 slave_addr, u16 reg, u16 reg_len, u8 data)
{
    int ret;
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    u8 paddr[3];
    
    osMutexAcquire(mutex_i2c, osWaitForever);
    
    
    if (2 == reg_len) {
        paddr[0] = ((reg >> 8) & 0xff);
        paddr[1] = reg & 0XFF;
        paddr[2] = data;
    }
    else {
        paddr[0] = (reg & 0XFF);
        paddr[1] = data;        
    }
    
    struct i2c_msg msgs[] = {
        {
            .addr = slave_addr,
            .flags = I2C_MASTER_WRITE,
            .len = reg_len + 1,
            .buf = paddr,
        }
    };
    
    ret = _i2c_transfer(ctx, msgs, 1);
    
    osMutexRelease(mutex_i2c);
    
    return ret;
}

static int _kdp520_i2c_read(
        struct core_device *core_d, u8 slave_addr, u16 reg, u16 reg_len, u8 *data)
{
	int ret;
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    u8 paddr[2];
    
    osMutexAcquire(mutex_i2c, osWaitForever);
    
    if (2 == reg_len) {
        paddr[0] = ((reg >> 8) & 0xff);
        paddr[1] = reg & 0XFF;
    }
    else {
        paddr[0] = (reg & 0XFF);
    }

	struct i2c_msg msgs[] = {
		{
			.addr = slave_addr,
			.flags = I2C_MASTER_WRITE,
			.len = reg_len,
			.buf = paddr,
		},
		{
			.addr = slave_addr,
			.flags = I2C_MASTER_READ,
			.len = 1,
			.buf = data,
		},
	};

	ret = _i2c_transfer(ctx, msgs, 2);

    osMutexRelease(mutex_i2c);
        
	return ret;
}
static int _kdp520_i2c_read_bytes(
        struct core_device *core_d, u8 slave_addr, u16 reg, u16 reg_len, u8 *data, u8 data_len)
{
	int ret;
    int i2c_id = core_d->uuid;
    struct kdp520_i2c_context *ctx = &i2c_ctx_s[i2c_id];

    u8 paddr[2];
    
    osMutexAcquire(mutex_i2c, osWaitForever);
    
    if (2 == reg_len) {
        paddr[0] = ((reg >> 8) & 0xff);
        paddr[1] = reg & 0XFF;
    }
    else {
        paddr[0] = (reg & 0XFF);
    }

	struct i2c_msg msgs[] = {
		{
			.addr = slave_addr,
			.flags = I2C_MASTER_WRITE,
			.len = reg_len,
			.buf = paddr,
		},
		{
			.addr = slave_addr,
			.flags = I2C_MASTER_READ,
			.len = data_len,
			.buf = data,
		},
	};

	ret = _i2c_transfer(ctx, msgs, 2);

    osMutexRelease(mutex_i2c);
    
	return ret;
}

#if CFG_I2C_0_ENABLE == YES
struct core_i2c_driver kdp520_i2c_0_driver = {
    .driver         = {
        .name       = "kdp520_i2c_0",
    },
    .probe          = _kdp520_i2c_probe,
    .remove         = _kdp520_i2c_remove,
    .core_dev       = &kdp520_i2c_0,
    
    .power_mgr      = {
        .power      = _kdp520_i2c_power,
        .suspend    = NULL,
        .resume     = NULL,
    }, 

    //.cfg_func     = (void*)&_i2c_set_params,
    .set_params     = _kdp520_i2c_set_params,    
    .init           = _kdp520_i2c_init,
    .reset          = _kdp520_i2c_reset,
    .write          = _kdp520_i2c_write,
    .read           = _kdp520_i2c_read,
	.readbytes		= _kdp520_i2c_read_bytes,	    
    .inited         = FALSE,
    .params         = { .bus_speed = 50000 },
};
#endif

#if CFG_I2C_1_ENABLE == YES
struct core_i2c_driver kdp520_i2c_1_driver = {
    .driver         = {
        .name       = "kdp520_i2c_1",
    },
    .probe          = _kdp520_i2c_probe,
    .remove         = _kdp520_i2c_remove,
    .core_dev       = &kdp520_i2c_1,
    
    .power_mgr      = {
        .power      = _kdp520_i2c_power,
        .suspend    = NULL,
        .resume     = NULL,
    }, 

    //.cfg_func     = (void*)&_i2c_set_params,
    .set_params     = _kdp520_i2c_set_params,    
    .init           = _kdp520_i2c_init,
    .reset          = _kdp520_i2c_reset,
    .write          = _kdp520_i2c_write,
    .read           = _kdp520_i2c_read,
	.readbytes		= _kdp520_i2c_read_bytes,
    .inited         = FALSE,
    .params         = { .bus_speed = 50000 },
};
#endif
#if CFG_I2C_2_ENABLE == YES
struct core_i2c_driver kdp520_i2c_2_driver = {
    .driver         = {
        .name       = "kdp520_i2c_2",
    },
    .probe          = _kdp520_i2c_probe,
    .remove         = _kdp520_i2c_remove,
    .core_dev       = &kdp520_i2c_2,
    
    .power_mgr      = {
        .power      = _kdp520_i2c_power,
        .suspend    = NULL,
        .resume     = NULL,
    }, 

    //.cfg_func     = (void*)&_i2c_set_params,
    .set_params     = _kdp520_i2c_set_params,    
    .init           = _kdp520_i2c_init,
    .reset          = _kdp520_i2c_reset,
    .write          = _kdp520_i2c_write,
    .read           = _kdp520_i2c_read,
	.readbytes		= _kdp520_i2c_read_bytes,
    .inited         = FALSE,
    .params         = { .bus_speed = 50000 },
};
#endif
#if CFG_I2C_3_ENABLE == YES
struct core_i2c_driver kdp520_i2c_3_driver = {
    .driver         = {
        .name       = "kdp520_i2c_3",
    },
    .probe          = _kdp520_i2c_probe,
    .remove         = _kdp520_i2c_remove,
    .core_dev       = &kdp520_i2c_3,
    
    .power_mgr      = {
        .power      = _kdp520_i2c_power,
        .suspend    = NULL,
        .resume     = NULL,
    }, 

    .set_params     = _kdp520_i2c_set_params,    
    .init           = _kdp520_i2c_init,
    .reset          = _kdp520_i2c_reset,
    .write          = _kdp520_i2c_write,
    .read           = _kdp520_i2c_read,
	.readbytes		= _kdp520_i2c_read_bytes,	
    .inited         = FALSE,
    .params         = { .bus_speed = 50000 },
};
#endif

static inline void _get_i2c_dev_drv(
        enum i2c_adap_id id, struct core_device **lpdev, struct core_i2c_driver **lpdrv)
{
    switch (id) {
#if CFG_I2C_0_ENABLE == YES        
    case I2C_ADAP_0:
        *lpdev = &kdp520_i2c_0;
        *lpdrv = &kdp520_i2c_0_driver;
        break;
#endif
#if CFG_I2C_1_ENABLE == YES    
    case I2C_ADAP_1:
        *lpdev = &kdp520_i2c_1;
        *lpdrv = &kdp520_i2c_1_driver;        
        break;
#endif
#if CFG_I2C_2_ENABLE == YES    
    case I2C_ADAP_2:
        *lpdev = &kdp520_i2c_2;
        *lpdrv = &kdp520_i2c_2_driver;        
        break;
#endif
#if CFG_I2C_3_ENABLE == YES    
    case I2C_ADAP_3:
        *lpdev = &kdp520_i2c_3;
        *lpdrv = &kdp520_i2c_3_driver;        
        break;
#endif    
    default:;
    }    
}

int kdp_drv_i2c_init(enum i2c_adap_id id, unsigned long bus_speed, BOOL force)
{
    int ret = -1;
    struct core_device *dev = NULL;
    struct core_i2c_driver *drv = NULL;
    _get_i2c_dev_drv(id, &dev, &drv);

    if (dev && drv)
    {        
        if ((drv->inited) && (force)) {
            drv->power_mgr.power(dev, FALSE);
            drv->remove(dev);
            drv->inited = FALSE;
        }
        
        if (!(drv->inited)) {
            drv->probe(dev);
            drv->power_mgr.power(dev, TRUE);
            drv->inited = TRUE;
        }
        {
            struct kdp520_i2c_params params;
            params.bus_speed = bus_speed;
            drv->set_params(dev, &params);
            drv->init(dev);
            drv->reset(dev); 
        }
        
        ret = 0;
    }
    
    return ret;
}

int kdp_drv_i2c_write(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 data)
{
    int ret = -1;
    
    struct core_device *dev = NULL;
    struct core_i2c_driver *drv = NULL;
    _get_i2c_dev_drv(id, &dev, &drv);
    
    ret = drv->write(dev, subaddr, reg, reg_len, data);    
    
    return ret;
}

int kdp_drv_i2c_read(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 *lpdata)
{
    int ret = -1;
    
    struct core_device *dev = NULL;
    struct core_i2c_driver *drv = NULL;
    _get_i2c_dev_drv(id, &dev, &drv);
    
    ret = drv->read(dev, subaddr, reg, reg_len, lpdata);    
    
    return ret;
}

int kdp_drv_i2c_read_bytes(enum i2c_adap_id id, u8 subaddr, u16 reg, u8 reg_len, u8 *lpdata, u8 data_len)
{
    int ret = -1;
    
    struct core_device *dev = NULL;
    struct core_i2c_driver *drv = NULL;
    _get_i2c_dev_drv(id, &dev, &drv);
    
    ret = drv->readbytes(dev, subaddr, reg, reg_len, lpdata, data_len);    
    
    return ret;
}

