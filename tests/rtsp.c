#if defined(HAVE_XPR_RTSP)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#  else
#pragma comment(lib, "libxpr.lib")
#  endif
#endif

static void segment(void* opaque, char* seg)
{
	printf("SEGMENT: [%s]\n", seg);
}

static void Test_OpenServer()
{
	XPR_RTSP_Init();
	for (int i = 0; i < 1000; i++) {
		int port = XPR_RTSP_Open(XPR_RTSP_PORT(2, 0, 0), "rtsp://0.0.0.0:554/akaa?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
		if (port < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", port);
		XPR_RTSP_Start(port);
		XPR_RTSP_Stop(port);
		XPR_RTSP_Close(port);
	}
	XPR_RTSP_Fini();
}

int main(int argc, char** argv)
{
	Test_OpenServer();
	system("pause");
	return 0;
}
#else
int main(int argc, char** argv)
{
	return 0;
}
#endif // defined(HAVE_XPR_RTSP)