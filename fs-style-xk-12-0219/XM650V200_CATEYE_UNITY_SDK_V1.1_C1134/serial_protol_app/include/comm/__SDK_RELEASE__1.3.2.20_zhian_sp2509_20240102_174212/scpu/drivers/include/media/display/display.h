#ifndef __DISPLAY_H__
#define __DISPLAY_H__


struct display_pen_info
{
    unsigned short color;
    unsigned int width;
};

struct display_driver_ops {
    void (*write_cmd)(unsigned int base, unsigned char data);
    void (*write_data)(unsigned int base, unsigned char data);
    unsigned int (*read_data)(unsigned int base);

    void (*write_cmd_data)(unsigned char cmd, unsigned char len, ...);
    void (*write_cmd_data_single)(unsigned char cmd, ...);
    void (*write_img_buf_data)(unsigned int len, void* buf);
    void (*read_data_with_buf)(unsigned char reg, unsigned char len, unsigned char* buf);
};

u32 kdp_display_get_buffer_addr(void);
int kdp_display_set_source(u32 src_addr, int src_dev_idx);
int kdp_display_set_pen_rgb565(unsigned short color, unsigned int pen_width);
int kdp_display_draw_line(u32 xs, u32 ys, u32 xe, u32 ye);
int kdp_display_draw_bitmap(u32 org_x, u32 org_y, u32 width, u32 height, void *buf);
int kdp_display_draw_rect(int x, int y, int width, int height);
int kdp_display_fill_rect(u32 org_x, u32 org_y, u32 width, u32 height);
int kdp_display_get_device_id(void);
int kdp_display_fresh(void);

#endif
