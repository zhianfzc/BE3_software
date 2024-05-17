#ifndef __FRAMEWORK_ERRNO_H__
#define __FRAMEWORK_ERRNO_H__


#define	KDP_FRAMEWORK_ERRNO_IO          0x00000001  /* i/o error */
#define	KDP_FRAMEWORK_ERRNO_CTX         0x00000002  /* pin_context error */
#define	KDP_FRAMEWORK_ERRNO_FD          0x00000003  /* fd error */
#define	KDP_FRAMEWORK_ERRNO_AGAIN       0x00000004  /* try again notification */
#define	KDP_FRAMEWORK_ERRNO_NOMEM       0x00000005  /* out of memory */
#define	KDP_FRAMEWORK_ERRNO_UNADDR      0x00000006  /* unknown address */
#define	KDP_FRAMEWORK_ERRNO_RESBUSY     0x00000007  /* resource busy */
#define	KDP_FRAMEWORK_ERRNO_INVALA      0x00000008  /* invalid parameter/argument */
#define KDP_FRAMEWORK_ERRNO_IOCTL       0x00000009  /* ioctl error */
#define KDP_FRAMEWORK_ERRNO_PROBE       0x0000000A  /* probe error */

#endif
