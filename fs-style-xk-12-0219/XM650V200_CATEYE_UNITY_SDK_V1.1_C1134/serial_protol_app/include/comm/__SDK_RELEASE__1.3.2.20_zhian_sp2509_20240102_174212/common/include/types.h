#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

//#if TARGET_SCPU
#define BS              0x08
#define ESC				27

#ifndef NULL
#define NULL    0
#endif

#ifndef ENABLE
#define ENABLE  1
#endif

#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif
#if 0
typedef int bool;
#define true 1
#define false 0
#endif
/* type define */
	typedef unsigned long long 		UINT64;
	typedef long long 				INT64;
	typedef	unsigned int			UINT32;
	typedef	int						INT32;
	typedef	unsigned short			UINT16;
	typedef	short					INT16;
	typedef unsigned char			UINT8;
	typedef char					INT8;
	typedef unsigned char			BOOL;

	typedef unsigned char           u8_t;
	typedef unsigned short          u16_t;
	typedef unsigned long           u32_t;
	typedef unsigned long long		u64_t;

	typedef unsigned char 			uchar;

    typedef char                    s8;
	typedef short                   s16;
    typedef int                     s32;
    typedef long long               s64;

    typedef unsigned char           u8;
	typedef unsigned short          u16;
    typedef unsigned int            u32;
    typedef unsigned long long      u64;

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef long ssize_t;
#endif

typedef INT8          INT8S;
typedef UINT8         INT8U;
typedef INT16         INT16S;
typedef UINT16        INT16U;
typedef INT32         INT32S;
typedef UINT32        INT32U;


typedef unsigned char                   byte;
typedef unsigned short                  word;
typedef unsigned long int               dword;

//#endif

#endif //TYPES_H
