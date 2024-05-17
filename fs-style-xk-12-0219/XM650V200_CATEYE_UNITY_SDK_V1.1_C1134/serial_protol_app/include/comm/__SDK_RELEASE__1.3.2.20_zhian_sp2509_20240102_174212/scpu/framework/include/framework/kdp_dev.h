#ifndef __KDP_DEV_H__
#define __KDP_DEV_H__


int kdp_dev_open(const char *dev_name);
int kdp_dev_close(int fd);
int kdp_dev_ioctl(int , unsigned int , void * );

#endif
