/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CHERRYUSB_CONFIG_H
#define CHERRYUSB_CONFIG_H

#include <board.h>
#include <rtthread.h>
#include <stdio.h>
#include <rthw.h>


#define CHERRYUSB_VERSION 0x000700

/* ================ USB common Configuration ================ */
#define USB_BASE            XM_USB0_BASE
#define DWC_USB_IRQ         IRQ_XM_USB0

#define CONFIG_USB_PRINTF   rt_kprintf

#define usb_malloc(size)    malloc(size)
#define usb_free(ptr)       free(ptr)

#define RT_USING_CACHE
#ifdef RT_USING_CACHE
#define  usb_dcache_flush(addr, len)        rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)addr, len)
#define  usb_dcache_invalidate(addr, len)   rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)addr, len)
#else
#define  usb_dcache_flush(addr, len)
#define  usb_dcache_invalidate(addr, len)
#endif

#ifndef CONFIG_USB_DBG_LEVEL
#define CONFIG_USB_DBG_LEVEL                USB_DBG_INFO
//#define CONFIG_USBDEV_SETUP_LOG_PRINT
#endif


/* data align size when use dma */
#ifndef CONFIG_USB_ALIGN_SIZE
#define CONFIG_USB_ALIGN_SIZE               32
#endif

/* ================= USB Device Stack Configuration ================ */

/* Ep0 max transfer buffer, specially for receiving data from ep0 out */
#define CONFIG_USBDEV_REQUEST_BUFFER_LEN    1024

/* ================ USB HOST Stack Configuration ================== */



#ifndef CONFIG_USBHOST_PSC_PRIO
#define CONFIG_USBHOST_PSC_PRIO             4
#endif
#ifndef CONFIG_USBHOST_PSC_STACKSIZE
#define CONFIG_USBHOST_PSC_STACKSIZE        2048
#endif

#define CONFIG_USBHOST_MAX_RHPORTS          1
#define CONFIG_USBHOST_MAX_EXTHUBS          1
#define CONFIG_USBHOST_MAX_EHPORTS          4
#define CONFIG_USBHOST_MAX_INTERFACES       6
#define CONFIG_USBHOST_MAX_INTF_ALTSETTINGS 1
#define CONFIG_USBHOST_MAX_ENDPOINTS        4

#define CONFIG_USBHOST_DEV_NAMELEN              16

/* Ep0 max transfer buffer */
#define CONFIG_USBHOST_REQUEST_BUFFER_LEN       512

#ifndef CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT
#define CONFIG_USBHOST_CONTROL_TRANSFER_TIMEOUT 500
#endif

/* ================ USB Host Port Configuration ==================*/
#define CONFIG_USBHOST_PIPE_NUM     11
#define CONFIG_USBHOST_CLASS_NUM    32
#define CONFIG_USBHOST_BULK_SIZE    127



#endif
