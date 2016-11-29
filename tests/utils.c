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

static void Test_xpr_foreach_s()
{
	xpr_foreach_s("hello world;are you ok?;", -1, ";", segment, NULL);
}

int main(int argc, char** argv)
{
	Test_xpr_foreach_s();
	system("pause");
	return 0;
}