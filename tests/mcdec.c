#if defined(HAVE_XPR_MCDEC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_mcdec.h>
#include <xpr/xpr_thread.h>
#include <Windows.h>

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#  else
#pragma comment(lib, "libxpr.lib")
#  endif
#endif

int avf_handler(void* opaque, int port, const XPR_AVFrame* avf)
{
	printf("opaque %p, port %x, %dx%d F %x\n", opaque, port, avf->width, avf->height, avf->format);
	return 0;
}


static void test_decode_264(void)
{
	char* buf = malloc(1024 * 16);
	size_t n = 0;
	FILE* fp;
	XPR_StreamBlock stb;
	fp = fopen("sample.264", "rb");
	if (fp) {
		memset(&stb, 0, sizeof(stb));
		while (1) {
			n = fread(buf, 1, 1024 * 16, fp);
			if (n <= 0)
				break;
			stb.buffer = buf;
			stb.bufferSize = 1024 * 16;
			stb.data = buf;
			stb.dataSize = (uint32_t)n;
			stb.codec = AV_FOURCC_H264;
			stb.flags = XPR_STREAMBLOCK_FLAG_TYPE_I;
			XPR_MCDEC_PushStreamBlock(XPR_MCDEC_PORT(1, 1), &stb);
			//Sleep(40);
		}
	}
}

int main(int argc, char** argv)
{
    XPR_MCDEC_Init();
	XPR_MCDEC_AddAVFrameHandler(0, avf_handler, NULL);
	test_decode_264();
	printf("exit\n");
    //XPR_MCDEC_Fini();
    return 0;
}
#else
int main(int argc, char** argv)
{
	return 0;
}
#endif