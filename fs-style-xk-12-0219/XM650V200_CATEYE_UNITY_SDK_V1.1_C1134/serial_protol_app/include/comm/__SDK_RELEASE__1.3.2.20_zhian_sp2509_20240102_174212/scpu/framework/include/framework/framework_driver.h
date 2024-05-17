#ifndef __DRIVER_H__
#define __DRIVER_H__


#include <stddef.h>
#include "types.h"
#include "framework/init.h"
#include "framework/kdp_list.h"
#include "framework/v2k_image.h"


typedef void (*fn_power_hook)(void);

struct pin_context {
    void *platform_data;
    void *driver_data;
    void *driver_this;
};
struct pin_context2 {
    void *platform_data;
};

struct driver_context {
    char name[20];

    int (*add) (struct driver_context *, struct pin_context *);
    int (*del) (struct driver_context *, struct pin_context *);

    struct kdp_list_node knode_to_buslist;
};

struct driver_context2 {
    int (*probe)(struct driver_context2 *, struct pin_context *);
    int (*reset)(struct driver_context2 *, struct pin_context *);
};

struct kdp520_csi2rx_context_lite {
    u32 ai_mem_addr;
    u32 ai_mem_addr_tmp;
    u32 vw_mem_addr;
    u32 ov_mem_addr;	
    u32 dp_mem_addr;	//for dsiplay
    u32 dpi2ahb_base;
};
struct core_device {
    const char *name;
    struct ioport_setting *ioport;
    int uuid; // universal unique id
    struct pin_context pin_ctx;
};
#define to_core_device(x) container_of((x), struct core_device, pin_ctx)

/* for core driver extension */
struct power_mgr_api { 
    int (*power)(struct core_device *, BOOL );/* power on or off the device */
    int (*suspend)(struct core_device *);   /* power management - suspend */
    int (*resume)(struct core_device *);    /* power management - resume */    
};

struct panel_driver {
    int (*init)(struct core_device **);
    int (*clear)(struct core_device **, u32 color);
    u16 (*read_did)(struct core_device **);
    void (*start)(struct core_device **);
    void (*stop)(struct core_device **);
    void (*preproc_rgb)(struct core_device **, u32 addr, u32 dest_addr);
    void (*posproc_rgb)(struct core_device **, u32 addr, u32 dest_addr);
    void (*preproc_nir)(struct core_device **, u32 addr, u32 dest_addr);   
    void (*posproc_nir)(struct core_device **, u32 addr, u32 dest_addr);
    void (*update)(struct core_device **, u32 addr);	
};

struct core_driver {
    int (*probe)(struct core_device *);     /* used to set up device-specific structures or resource */
    int (*remove)(struct core_device *);    /* used to release device-specfic structures or resource */
    struct core_device *core_dev;
    struct driver_context driver;
    //struct power_mgr_api power_mgr;
    void *cfg_func;
};
#define to_core_driver(drv)	(container_of((drv), struct core_driver, driver))

struct kdp520_i2c_params {
    unsigned long bus_speed;
};
struct core_i2c_driver {
    int (*probe)(struct core_device *);     /* used to set up device-specific structures or resource */
    int (*remove)(struct core_device *);    /* used to release device-specfic structures or resource */
    struct core_device *core_dev;
    struct driver_context driver;
    struct power_mgr_api power_mgr;
    void *cfg_func;

    int (*set_params)(struct core_device *, struct kdp520_i2c_params *);
    int (*init)(struct core_device *);
    int (*reset)(struct core_device *);
    int (*write)(struct core_device *, u8 , u16 , u16 , u8 );
    int (*read)(struct core_device *, u8 , u16 , u16 , u8 *);
    int (*readbytes)(struct core_device *, u8 , u16 , u16 , u8 *, u8);
    
    BOOL inited;
    struct kdp520_i2c_params params;
};

typedef struct _lcm_custom_pinmux {
    u32 pin_cs;
    u32 pin_te;
    u32 pin_rs;
    u32 pin_rst;
} lcm_custom_pinmux;

struct display_driver {
    // same as core_driver
    int (*probe)(struct core_device **);     /* used to set up device-specific struct ures or resource */
    int (*remove)(struct core_device **);    /* used to release device-specfic structures or resource */
    struct core_device *core_dev;
    struct driver_context driver;
    struct power_mgr_api power_mgr;
    void *cfg_func;

    void *ctrller_ops;
    lcm_custom_pinmux* custom_pinmux;

    struct video_input_params vi_params;
    u32 fb_size;
	u32 type;
    u32 base;
    u16 display_id;
    struct panel_driver *panel;
    int (*attach_panel)(struct core_device **, struct panel_driver *panel);
    int (*set_params)(struct core_device **, struct video_input_params *);
    int (*get_params)(struct core_device **, struct video_input_params *);
    int (*set_camera)(struct core_device **);
//    int (*buffer_init)(struct core_device *, struct video_input_params *);
    u32 (*get_buffer)(struct core_device **);
    int (*init)(struct core_device **);
    int (*start)(struct core_device **);
    int (*stop)(struct core_device **);
    int (*set_source)(struct core_device **, u32, int);
    int (*set_pen)(struct core_device **, u16 , u32 );	
    int (*draw_rect)(struct core_device **, int, int, u32, u32);
    int (*draw_line)(struct core_device **, u32, u32, u32, u32);
    int (*fill_rect)(struct core_device **, u32, u32, u32, u32);
    int (*draw_bitmap)(struct core_device **, u32, u32, u32, u32, void *);
    int (*fresh)(struct core_device **);
};

struct touch_panel_driver {
    int (*init)(struct core_device *);
    u16 (*read_did)(struct core_device *);
    void (*start)(struct core_device *);
    void (*stop)(struct core_device *);

    void (*get_raw_data)(struct core_device *, u8);
    void (*state_handler)(struct core_device *, void*);
    void (*set_inactive)(void);
    int (*get_active)(void);
};

struct touch_driver {
    int (*probe)(struct core_device *);
    int (*remove)(struct core_device *);
    struct core_device *core_dev;
    struct driver_context driver;
    struct power_mgr_api power_mgr;
    void *cfg_func;
    void *ctrller_ops;

    u16 device_id;
    struct touch_panel_driver *panel;
    int (*attach_panel)(struct core_device *, struct touch_panel_driver *panel);
    int (*init)(struct core_device *);
    int (*start)(struct core_device *);
    int (*stop)(struct core_device *);
    BOOL (*is_started)(struct core_device *);
    void (*set_hook)(struct core_device *, fn_power_hook , fn_power_hook );
    
    u16 count;
    u16 x[10];
    u16 y[10];

    u16 x_range_max;
    u16 y_range_max;
    u8 inverse_x_axis;
    u8 inverse_y_axis;
    fn_power_hook cb_power_on;
    fn_power_hook cb_power_off;
};

////////////////////////////
/* specific platform data */
////////////////////////////
/* i2c platform data */
struct i2c_platform_data {
    unsigned long bus_speed;
};


extern struct driver_context* find_driver(char *);
extern struct ioport_setting *driver_get_ioport_setting(struct core_device *, unsigned int);

extern int drivers_init(void);
extern int driver_register(struct driver_context *, struct pin_context *);
extern void driver_unregister(struct driver_context *);
//extern int driver_core_register(struct core_driver *);
//extern void driver_core_unregister(struct core_driver *);
extern int driver_core_register(void *);
extern void driver_core_unregister(void *);

void* framework_drv_get_drvdata(const struct pin_context *);
void  framework_drv_set_drvdata(struct pin_context *, void *data);
void* framework_drv_get_drv(const struct pin_context *pin_ctx);
void  framework_drv_set_drv(struct pin_context *pin_ctx, void *this);

#define DRIVER_SETUP(__driver, __setup_level, __register, __unregister, ...) \
static int INITTEXT __driver##_entr(void) { \
    return __register(&(__driver) , ##__VA_ARGS__); \
} \
ARRANGE_ENTR_RO_SECTION(__driver##_entr, __setup_level); \
static void FINITEXT __driver##_exit(void) { \
    __unregister(&(__driver) , ##__VA_ARGS__); \
} \
ARRANGE_EXIT_RO_SECTION(__driver##_exit);

#define DRIVER_SETUP_2(__driver, __setup_level, __register, __unregister, ...) \
static int INITTEXT __driver##_entr(void) { \
    return __register((struct core_driver*)(&(__driver)), ##__VA_ARGS__); \
} \
ARRANGE_ENTR_RO_SECTION(__driver##_entr, __setup_level); \
static void FINITEXT __driver##_exit(void) { \
    __unregister((struct core_driver*)(&(__driver)), ##__VA_ARGS__); \
} \

#define KDP_CORE_DRIVER_SETUP(__core_driver, __setup_level) \
    DRIVER_SETUP(__core_driver, __setup_level, driver_core_register, driver_core_unregister)
#define KDP_CORE_DRIVER_SETUP_2(__core_driver, __setup_level) \
    DRIVER_SETUP_2(__core_driver, __setup_level, driver_core_register, driver_core_unregister)

int framework_drv_init(void);

#endif /* __DEVICE_H__ */
