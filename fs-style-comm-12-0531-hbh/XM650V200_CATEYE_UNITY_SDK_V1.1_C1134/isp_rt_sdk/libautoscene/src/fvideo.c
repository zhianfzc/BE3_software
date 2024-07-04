#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include "isp_print.h"
#include "autoscene_para.h"
#ifdef USE_LIBFVIDEO
#include "mpi_camera.h"
#endif

ISP_S32 fvideo_get_capture_size(ISP_DEV IspDev, ISP_S32 *ps32CaptureSize)
{
	if(ps32CaptureSize == NULL)
	{
		return ISP_FAILURE;
	}
	#ifdef USE_LIBFVIDEO
		ISP_S32 s32CaptureSize;
		ISPCHECK_RET(XM_MPI_CAM_GetCaptureSize(IspDev, &s32CaptureSize), "XM_MPI_CAM_GetCaptureSize");
		*ps32CaptureSize = s32CaptureSize;
	#else
		*ps32CaptureSize = 0;
	#endif
	return ISP_SUCCESS;
}
