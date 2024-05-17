#ifndef __KL520_SYS_H__
#define __KL520_SYS_H__

#include "types.h"
#include "board_kl520.h"

#define MAX(x,y)            ((x)<(y)?(y):(x))
#define MIN(x,y)            ((x)>(y)?(y):(x))
#define ABS(a)              (((a)>=0)?(a):(-(a)))

/*Engineering Settings*/
#define CALC_CAMERA_FPS     (NO)
#define CALC_FDFR_MS_ONCE   (NO)
#define CALC_FDFR_FPS       (NO)
#define CALC_DISPLAY_FPS    (NO)

#define MEASURE_RECOGNITION (NO)
#define MEASURE_WITH_TOUCH  (NO)

#define UART_INIT_AFTER_OTA (NO)

#define CALIBRA_REG_OFFSET  (YES)

//////////////////////////
/* Engineering mode API */
//////////////////////////

#endif
