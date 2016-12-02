#if defined(WIN32) || defined(_WIN32)
#include <xpr/xpr_config_win32.h>
#else
#include <xpr/xpr_config.h>
#endif // defined(WIN32) || defined(_WIN32)

#if defined(HAVE_XPR_RTSP)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_sys.h>
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
	int serverPort = XPR_RTSP_PORT(2, 0, 0);
	for (int i = 0; i < 10; i++) {
		int err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/akaa?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
		if (err < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", serverPort);
		XPR_RTSP_Start(serverPort);
		XPR_RTSP_Stop(serverPort);
		XPR_RTSP_Close(serverPort);
	}
	XPR_RTSP_Fini();
}

static void Test_StartStop()
{
	XPR_RTSP_Init();
	int serverPort = XPR_RTSP_PORT(2, 0, 0);
	int streamPort = XPR_RTSP_PORT(2, 1, 0);
	int err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/live/0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
	if (err == XPR_ERR_OK) {
		int err = XPR_RTSP_Open(streamPort, "uri:///live-1?track=1&mime=video/H264&track=2&mime=audio/G711");
		if (err < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", err);
		err = XPR_RTSP_Start(streamPort);
		fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		err = XPR_RTSP_Start(serverPort);
		fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		XPR_ThreadSleep(1000000000);
		XPR_RTSP_Stop(streamPort);
		XPR_RTSP_Stop(serverPort);
		XPR_RTSP_Close(streamPort);
		XPR_RTSP_Close(serverPort);
	}
	XPR_RTSP_Fini();
}

struct FrameInfo {
	uint32_t offset;
	uint32_t length;
};

static void Test_H264Video()
{
	struct FrameInfo fi = { 0 };
	XPR_StreamBlock stb = { 0 };

	XPR_RTSP_Init();
	int serverPort = XPR_RTSP_PORT(2, 0, 0);
	int streamPort = XPR_RTSP_PORT(2, 1, 0);
	int err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/live/0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
	if (err == XPR_ERR_OK) {
		int err = XPR_RTSP_Open(streamPort, "uri:///live-1?track=1&mime=video/H264&track=2&mime=audio/G711");
		if (err < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", err);
		err = XPR_RTSP_Start(streamPort);
		fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		err = XPR_RTSP_Start(serverPort);
		fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		//
		XPR_File* f1 = XPR_FileOpen("D:\\Home\\Varphone\\Videos\\3516a.264", "rb");
		XPR_File* f2 = XPR_FileOpen("D:\\Home\\Varphone\\Videos\\3516a.264.idx", "rb");

		memset(&stb, 0, sizeof(stb));
		stb.bufferSize = 1024 * 1024;
		stb.buffer = malloc(stb.bufferSize);
		stb.data = stb.buffer;
		stb.dataSize = 0;
		stb.codec = AV_FOURCC_H264;
		stb.track = 1;
		while (1) {
			int n1 = XPR_FileRead(f2, &fi, sizeof(fi));
			if (n1 != sizeof(fi)) {
				XPR_FileSeek(f1, 0, XPR_FILE_SEEK_SET);
				XPR_FileSeek(f2, 0, XPR_FILE_SEEK_SET);
				continue;
			}
			XPR_FileSeek(f1, fi.offset, XPR_FILE_SEEK_SET);
			int n2 = XPR_FileRead(f1, stb.data, fi.length);
			if (n2 != fi.length)
				continue;
			stb.dataSize = fi.length;
			stb.pts = XPR_SYS_GetCTS();
			XPR_RTSP_PushData(streamPort, &stb);
			//printf("send stream %d, %d\n", fi.offset, fi.length);
			XPR_ThreadSleep(40000);
		}
		XPR_RTSP_Stop(streamPort);
		XPR_RTSP_Stop(serverPort);
		XPR_RTSP_Close(streamPort);
		XPR_RTSP_Close(serverPort);
	}
	XPR_RTSP_Fini();
}

int main(int argc, char** argv)
{
	//Test_OpenServer();
	//Test_StartStop();
	Test_H264Video();
	system("pause");
	return 0;
}
#else
int main(int argc, char** argv)
{
	return 0;
}
#endif // defined(HAVE_XPR_RTSP)