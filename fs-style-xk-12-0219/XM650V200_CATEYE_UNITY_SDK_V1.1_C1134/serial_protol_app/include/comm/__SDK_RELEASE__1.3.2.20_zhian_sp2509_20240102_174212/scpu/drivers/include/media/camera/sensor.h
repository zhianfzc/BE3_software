#ifndef __SENSOR_H__
#define __SENSOR_H__


#include "framework/framework_driver.h"
#include "kdp520_i2c.h"

struct sensor_device {
    enum i2c_adap_id i2c_id;
    struct pin_context pin_ctx;         /* the pin_context structure */ // sys_camera_device->control
    unsigned short addr;                /* chip address - NOTE: 7bit */
    int device_id;
};
#define to_sensor_device(d) container_of(d, struct sensor_device, pin_ctx)

//struct sensor_device;
struct sensor_driver {
    int (*probe)(struct sensor_device *);
    int (*remove)(struct sensor_device *);

    struct core_device *core_dev;
    struct driver_context driver;
};
#define to_sensor_driver(__adap__) container_of(__adap__, struct sensor_driver, driver)

int sensor_driver_register(struct sensor_driver *);
void sensor_driver_unregister(struct sensor_driver *);

#define KDP_SENSOR_DRIVER_SETUP(__sensor_driver) \
    DRIVER_SETUP(__sensor_driver, 3, sensor_driver_register, sensor_driver_unregister)

#endif 
