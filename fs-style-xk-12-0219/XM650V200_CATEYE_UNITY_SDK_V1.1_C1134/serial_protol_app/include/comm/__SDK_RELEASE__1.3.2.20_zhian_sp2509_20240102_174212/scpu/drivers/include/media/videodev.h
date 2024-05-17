#ifndef __VIDEODEV_H__
#define __VIDEODEV_H__


#include "framework/bitops.h"
#include "framework/framework_driver.h"
#include "framework/v2k.h"

struct v2k_pin_obj {
    struct pin_context *pin_ctx;
};

struct video_device_context {
    const struct v2k_dev_operations *dev_ops;
    const struct v2k_ioctl_ops *ioctl_ops;      /* ioctl callbacks */
    struct pin_context pin_ctx;              /* v2k pin_context */
};

struct video_device_context *video_devdata(struct v2k_dev_handle *);

int video_device_register(struct video_device_context *);//, int );
void video_device_release(struct video_device_context *);


#endif 
