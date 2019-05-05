#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HAVE_CONFIG_H 1
#include <xpr/xpr.h>

//#if defined(WIN32) || defined(_WIN32)
//#include <xpr/xpr_config_win32.h>
//#else
//#include <xpr/xpr_config.h>
//#endif // defined(WIN32) || defined(_WIN32)

#if defined(HAVE_XPR_RTSP)
//#include <xpr/xpr_atomic.h>
//#include <xpr/xpr_errno.h>
//#include <xpr/xpr_fifo.h>
//#include <xpr/xpr_file.h>
//#include <xpr/xpr_mem.h>
//#include <xpr/xpr_rtsp.h>
//#include <xpr/xpr_sys.h>
//#include <xpr/xpr_thread.h>
//#include <xpr/xpr_utils.h>

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#  else
#pragma comment(lib, "libxpr.lib")
#  endif
#endif

#if 0
static void segment(void* opaque, char* seg)
{
	printf("SEGMENT: [%s]\n", seg);
}
#endif

#ifdef TEST_OPEN_SERVER
static void Test_OpenServer()
{
	int i;
	int err;
	int serverPort;

	XPR_RTSP_Init();
	serverPort = XPR_RTSP_PORT(2, 0, 0);
	for (i = 0; i < 10; i++) {
		err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/akaa?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
		if (err < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", serverPort);
		XPR_RTSP_Start(serverPort);
		XPR_RTSP_Stop(serverPort);
		XPR_RTSP_Close(serverPort);
	}
	XPR_RTSP_Fini();
}
#endif

#ifdef TEST_START_STOP
static void Test_StartStop()
{
	int err;
	int serverPort;
	int streamPort;

	XPR_RTSP_Init();
	serverPort = XPR_RTSP_PORT(2, 0, 0);
	streamPort = XPR_RTSP_PORT(2, 1, 0);
	err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/live/0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
	if (err == XPR_ERR_OK) {
		err = XPR_RTSP_Open(streamPort, "uri:///live-1?vsrc=0&vql=10&asrc=0&aql=10&track=1&mime=video/H264&track=2&mime=audio/G711");
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
#endif

struct FrameInfo {
	uint32_t offset;
	uint32_t length;
};

static void Test_H264Video(const char* path)
{
	int err;
	int n1, n2;
	int serverPort;
	int streamPort;
	struct FrameInfo fi = { 0 };
	XPR_StreamBlock stb = { 0 };
	char h264FileName[256];
	char h264IndexFileName[256];
	int64_t pts = 0;

	sprintf(h264FileName, "%s", path);
	sprintf(h264IndexFileName, "%s.idx", path);

	XPR_RTSP_Init();
	serverPort = XPR_RTSP_PORT(2, 0, 0);
	streamPort = XPR_RTSP_PORT(2, 1, 0);
	err = XPR_RTSP_Open(serverPort, "rtsp://0.0.0.0:554/live/0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
	if (XPR_IS_ERROR(err)) {
		printf("XPR_RTSP_Open() failed, %x\n", err);
	}
	else if (err == XPR_ERR_OK) {
		err = XPR_RTSP_Open(streamPort, "uri:///live-1?track=1&mime=video/H264&track=2&mime=audio/G711");
		if (err < 0)
			fprintf(stderr, "XPR_RTSP_Open() failed, errno: %x\n", err);
		fprintf(stderr, "RTSP [%d] URL: %s\n", streamPort, "rtsp://0.0.0.0/live-1");
		err = XPR_RTSP_Start(streamPort);
		//fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		err = XPR_RTSP_Start(serverPort);
		//fprintf(stderr, "XPR_RTSP_Start() failed, errno: %x\n", err);
		//
		XPR_File* f1 = XPR_FileOpen(h264FileName, "rb");
		XPR_File* f2 = XPR_FileOpen(h264IndexFileName, "rb");

		memset(&stb, 0, sizeof(stb));
		stb.bufferSize = 1024 * 1024;
		stb.buffer = malloc(stb.bufferSize);
		stb.data = stb.buffer;
		stb.dataSize = 0;
		stb.codec = AV_FOURCC_H264;
		stb.track = 1;
		while (1) {
			n1 = XPR_FileRead(f2, (uint8_t*)&fi, sizeof(fi));
			if (n1 != sizeof(fi)) {
				XPR_FileSeek(f1, 0, XPR_FILE_SEEK_SET);
				XPR_FileSeek(f2, 0, XPR_FILE_SEEK_SET);
				continue;
			}
			if (fi.length <= 0)
				continue;
			XPR_FileSeek(f1, fi.offset, XPR_FILE_SEEK_SET);
			n2 = XPR_FileRead(f1, stb.data, fi.length);
			if (n2 != fi.length)
				continue;
			stb.dataSize = fi.length;
			stb.pts = pts;
			XPR_RTSP_PushData(streamPort, &stb);
			//fprintf(stderr, "### Send Frame: O = %12d, L = %8d, PTS = %14lld\n",
			//	fi.offset, fi.length, stb.pts);
			XPR_ThreadSleep(38000);
			pts += 40000;
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
#ifdef TEST_OPEN_SERVER
	Test_OpenServer();
#endif
#ifdef TEST_START_STOP
	Test_StartStop();
#endif
	Test_H264Video(argv[1]);
	system("pause");
	return 0;
}
#else
int main(int argc, char** argv)
{
	return 0;
}
#endif // defined(HAVE_XPR_RTSP)
