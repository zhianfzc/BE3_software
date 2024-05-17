/*
 * Standard image size definitions
 */
#ifndef __V2K_IMAGE_H__
#define __V2K_IMAGE_H__

#include "v2k.h"

#define QVGA_LANDSCAPE_WIDTH    320
#define QVGA_LANDSCAPE_HEIGHT   240
#define QVGA_PORTRAIT_WIDTH     240
#define QVGA_PORTRAIT_HEIGHT    320

#define TFT43_WIDTH             480
#define TFT43_HEIGHT            272

#define VGA_LANDSCAPE_WIDTH     640
#define VGA_LANDSCAPE_HEIGHT    480
#define VGA_PORTRAIT_WIDTH      480
#define VGA_PORTRAIT_HEIGHT     640

#define HD_WIDTH                1280
#define HD_HEIGHT               720

#define FHD_WIDTH               1920
#define FHD_HEIGHT              1080

#define HMX_RICA_WIDTH          864
#define HMX_RICA_HEIGHT         491

#define QVGA_WIDTH              1280
#define QVGA_HEIGHT             960

#define UGA_WIDTH               1600
#define UGA_HEIGHT              1200

#define SC132_FULL_RES_WIDTH    1080
#define SC132_FULL_RES_HEIGHT   1280


enum image_input_format {
    image_input_format_rgb565 = 0,
    image_input_format_rgb555,
    image_input_format_rgb444,
    image_input_format_rgb24,
    image_input_format_ycbcr422,
    image_input_format_ycbcr420,
    image_input_format_palette_8,
    image_input_format_palette_4,
    image_input_format_palette_2,
    image_input_format_palette_1,
};

struct sensor_datafmt_info {
    unsigned int fourcc;
    enum v2k_colorspace colorspace;
};

struct sensor_win_size {
    unsigned int width;
    unsigned int height;
};

// move this to a better place
struct sensor_init_seq {
    unsigned short addr;
    unsigned char value;
}__attribute__((packed));


struct video_input_params {

    unsigned int    src_fmt;
    unsigned int    src_type;
    unsigned short  src_cam_idx;    //input
    unsigned short  src_width;      //input
    unsigned short  src_height;     //input
    
    unsigned short  dp_area_x;      //display area x relative to src_width
    unsigned short  dp_area_y;      //display area y relative to src_height
    unsigned short  dp_area_w;      //display area width relative to src_width 
    unsigned short  dp_area_h;      //display area height relative to src_height
    unsigned short  dp_out_w;       //display width
    unsigned short  dp_out_h;       //display height

    unsigned short  panel_in_w;    //screen in
    unsigned short  panel_in_h;    //screen in
    unsigned short  panel_out_w;   //screen out
    unsigned short  panel_out_h;   //screen out
};

struct kdp_rect {
    unsigned short start_x;
    unsigned short start_y;
    unsigned short end_x;
    unsigned short end_y;
};

int calc_framesize(
        unsigned short frame_width, 
        unsigned short frame_height, 
        unsigned int input_fmt);

#endif
