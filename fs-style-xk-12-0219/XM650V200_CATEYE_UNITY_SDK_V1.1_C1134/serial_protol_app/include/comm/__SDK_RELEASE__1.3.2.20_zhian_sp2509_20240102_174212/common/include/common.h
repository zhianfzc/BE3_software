#ifndef COMMON_H
#define COMMON_H

#include "board_cfg.h"

//#define DEVICE_ID           0x520       // Kneron device id for KL520
//#define FW_VERSION          0x00090505  // 0, 9, 5, 5 major, minor, patch, edit

//#define FID_THRESHOLD       (0.425f)

#ifdef CFG_MAX_USER
#define MAX_USER            (CFG_MAX_USER)  // MAX user count in DB
#else
#define MAX_USER            (100)  // MAX user count in DB
#endif
#define MAX_FID             (5)   // MAX face count for one user

#define MAX_LEN_USERNAME    (32)

//#define ALL_USER_UID        (0)

#endif
