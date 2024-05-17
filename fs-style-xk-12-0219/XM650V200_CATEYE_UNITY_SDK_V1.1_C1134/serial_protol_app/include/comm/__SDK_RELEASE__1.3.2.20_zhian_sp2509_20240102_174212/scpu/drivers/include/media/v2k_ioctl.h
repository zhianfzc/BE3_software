#ifndef __INTERNAL_V2K_IOCTL_H__
#define __INTERNAL_V2K_IOCTL_H__


#include "framework/v2k.h"
#include "framework/v2k_ioctl.h"


struct v2k_ioctl_ops {
    int (*vioc_querycap)     (struct v2k_dev_handle *handle, void *priv, struct v2k_capability *cap);
    int (*vioc_g_fmt_vid_cap)(struct v2k_dev_handle *handle, void *priv, struct v2k_format *f);
    int (*vioc_s_fmt_vid_cap)(struct v2k_dev_handle *handle, void *priv, struct v2k_format *f);
    int (*vioc_reqbufs)      (struct v2k_dev_handle *handle, void *priv, struct v2k_requestbuffers *rb);
    int (*vioc_querybuf)     (struct v2k_dev_handle *handle, void *priv, struct v2k_buffer *b);
    int (*vioc_qbuf)         (struct v2k_dev_handle *handle, void *priv, struct v2k_buffer *b);
    int (*vioc_dqbuf)        (struct v2k_dev_handle *handle, void *priv, struct v2k_buffer *b);
    int (*vioc_streamon)     (struct v2k_dev_handle *handle, void *priv, unsigned long arg);
    int (*vioc_streamoff)    (struct v2k_dev_handle *handle, void *priv, unsigned long arg);
};

long video_default_ioctl(struct v2k_dev_handle *handle, unsigned int cmd, void* arg);

#endif
