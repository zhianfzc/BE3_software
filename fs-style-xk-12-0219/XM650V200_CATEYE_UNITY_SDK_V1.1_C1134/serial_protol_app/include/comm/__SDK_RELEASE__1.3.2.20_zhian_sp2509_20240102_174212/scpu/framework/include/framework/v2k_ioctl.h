#ifndef __V2K_IOCTL_H__
#define __V2K_IOCTL_H__


#define _DEV_IOCTL_READ       1U
#define _DEV_IOCTL_WRITE      2U
#define _DEV_IOCTL_RW         (_DEV_IOCTL_READ | _DEV_IOCTL_WRITE)

#define _DEV_IOCTL_DIRBITS    2
#define _DEV_IOCTL_NUMBITS    8
#define _DEV_IOCTL_SIZEBITS   22

#define _DEV_IOCTL_DIRSHIFT   0
#define _DEV_IOCTL_NUMSHIFT   (_DEV_IOCTL_DIRSHIFT + _DEV_IOCTL_DIRBITS)
#define _DEV_IOCTL_SIZESHIFT  (_DEV_IOCTL_NUMSHIFT + _DEV_IOCTL_NUMBITS)

#define __DEV_IOCTL_DEF(__dir, __nr, __size) (((__dir)  << _DEV_IOCTL_DIRSHIFT) | \
                                              ((__nr)   << _DEV_IOCTL_NUMSHIFT) | \
                                              ((__size) << _DEV_IOCTL_SIZESHIFT))

#define V2K_VIOC_QUERYCAP   __DEV_IOCTL_DEF(_DEV_IOCTL_READ, (0),(sizeof(struct v2k_capability)))
#define V2K_VIOC_G_FMT      __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (1),(sizeof(struct v2k_format)))
#define V2K_VIOC_S_FMT      __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (2),(sizeof(struct v2k_format)))
#define V2K_VIOC_REQBUFS    __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (3),(sizeof(struct v2k_requestbuffers)))    
#define V2K_VIOC_QUERYBUF   __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (4),(sizeof(struct v2k_buffer)))
#define V2K_VIOC_QBUF       __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (5),(sizeof(struct v2k_buffer)))
#define V2K_VIOC_DQBUF      __DEV_IOCTL_DEF(_DEV_IOCTL_RW,   (6),(sizeof(struct v2k_buffer)))
#define V2K_VIOC_STREAMON   __DEV_IOCTL_DEF(_DEV_IOCTL_WRITE,(7),(sizeof(int)))
#define V2K_VIOC_STREAMOFF  __DEV_IOCTL_DEF(_DEV_IOCTL_WRITE,(8),(sizeof(int)))


#endif /* _V2K_IOCTL_H */
