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


static void test_decode_264(const char* file)
{
	XPR_MCDEC_AddAVFrameHandler(XPR_MCDEC_PORT(1, 1), avf_handler, NULL);
	char* buf = malloc(1024 * 16);
	size_t n = 0;
	FILE* fp;
	XPR_StreamBlock stb;
	fp = fopen(file, "rb");
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
			Sleep(40);
		}
	}
}

static int stb_handler(void* opaque, int port, const XPR_StreamBlock* stb)
{
	printf("%p %d %p\n", opaque, port, stb);
	FILE* fp = fopen("C:\\Users\\Varphone\\Videos\\out.jpg", "w+b");
	if (fp) {
		fwrite(stb->data, 1, stb->dataSize, fp);
		fclose(fp);
	}
	return 0;
}
static void test_bmpenc(const char* yuvfile)
{
	XPR_MCDEC_AddStreamBlockHandler(XPR_MCDEC_SPEC_JPGENC_PORT, stb_handler, NULL);
	FILE* fp = fopen(yuvfile, "rb");
	XPR_AVFrame avf;
	memset(&avf, 0, sizeof(avf));
	avf.format = XPR_AV_PIX_FMT_YUV420P;
	avf.width = 1024;
	avf.height = 600;
	avf.pitches[0] = 1024;
	avf.pitches[1] = 512;
	avf.pitches[2] = 512;
	avf.datas[0] = malloc(1024 * 600);
	avf.datas[1] = malloc(1024 * 600 / 4);
	avf.datas[2] = malloc(1024 * 600 / 4);
	fread(avf.datas[0], 1, 1024 * 600, fp);
	fread(avf.datas[1], 1, 1024 * 600 / 4, fp);
	fread(avf.datas[2], 1, 1024 * 600 / 4, fp);
	printf("%d\n", XPR_MCDEC_PushAVFrame(XPR_MCDEC_SPEC_JPGENC_PORT, &avf));
}

int main(int argc, char** argv)
{
	XPR_MCDEC_Config(XPR_MCDEC_CFG_LOG_LEVEL, 64, 0);
    XPR_MCDEC_Init();
	test_decode_264(argv[1]);
	test_bmpenc(argv[1]);
	while (1) {
		XPR_ThreadSleep(1000000);
	}
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