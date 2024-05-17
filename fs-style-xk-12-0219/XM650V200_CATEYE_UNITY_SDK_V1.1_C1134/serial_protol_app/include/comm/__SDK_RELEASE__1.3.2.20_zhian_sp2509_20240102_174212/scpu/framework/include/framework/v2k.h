#ifndef __V2K_H__
#define __V2K_H__


#define V2K_CAP_VIDEO_CAPTURE   0x00000001  /* Is a video capture device */
#define V2K_CAP_STREAMING       0x00000002  /* can stream on/off */
#define V2K_CAP_DEVICE_CAPS     0x00000004  /* can query capabilities */

enum v2k_field {
    V2K_FIELD_ANY               = 0, 
    V2K_FIELD_NONE              = 1, /* this device has no fields ... */
    V2K_FIELD_INTERLACED        = 4, /* both fields interlaced */
    V2K_FIELD_INTERLACED_DEPTH  = 0x10, 
};

enum v2k_colorspace {
    V2K_COLORSPACE_RGB          = 0,
    V2K_COLORSPACE_YUV          = 1,
    V2K_COLORSPACE_RAW          = 2,
};

struct v2k_rect {
    int left;
    int top;
    unsigned int width;
    unsigned int height;
};

struct v2k_fract {
    unsigned int numerator;
    unsigned int denominator;
};

struct v2k_capability {
    char driver[16];
    char desc[16];
    unsigned int version;
    unsigned int capabilities;
    int device_id;
};

struct v2k_format {
    unsigned int width;
    unsigned int height;
    unsigned int pixelformat;    /* fourcc */
    unsigned int field;          /* enum v2k_field */
    unsigned int bytesperline;   /* for padding, zero if unused */
    unsigned int sizeimage;
    unsigned int colorspace;     /* enum v2k_colorspace */
};

struct v2k_buffer {
    unsigned int type;
    unsigned int index;
    unsigned int flags;
    unsigned int field;
    unsigned int length;
    unsigned int offset;
    unsigned long timestamp;    
};

struct v2k_requestbuffers {
    unsigned int count;
};

struct v2k_dev_operations;
struct v2k_dev_handle {
    int i_rdev;
    void *private_data;
    const struct v2k_dev_operations *dev_ops;
};

struct v2k_dev_operations {
    int (*open)(struct v2k_dev_handle *);
    int (*close)(struct v2k_dev_handle *);
    //int (*release)(struct v2k_dev_handle *);
    long (*ioctl)(struct v2k_dev_handle *, unsigned int, void * );    
};

#define v2k_fourcc(a, b, c, d) \
    ((unsigned int)(a) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))

#define V2K_PIX_FMT_YCBCR   v2k_fourcc('Y', 'B', 'Y', 'R')
#define V2K_PIX_FMT_RGB565  v2k_fourcc('R', 'G', 'B', 'P')
#define V2K_PIX_FMT_RAW10   v2k_fourcc('R', 'A', '1', '0')
#define V2K_PIX_FMT_RAW8    v2k_fourcc('R', 'A', 'W', '8')

#define V2K_TYPE_STATIC   	0
#define V2K_TYPE_DYNAMIC    1

#endif 
