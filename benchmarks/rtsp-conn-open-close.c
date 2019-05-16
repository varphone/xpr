#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_utils.h>

#define XPR_ERR_ASSERT(err)                                                    \
    if (err != XPR_ERR_OK) {                                                   \
        printf("%s:%d, XPR_ErrorCode = 0x%08X\n", __FILE__, __LINE__, err);    \
        abort();                                                               \
    }

const char* urls[] = {
    "rtsp://127.0.0.1:8554/example.aac",
};

void benchmark(void)
{
    XPR_RTSP_Init();
    int ret;
    int mgrPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    ret = XPR_RTSP_Open(mgrPort, "dummy://any");
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Start(mgrPort);
    XPR_ERR_ASSERT(ret);
    uint32_t n = 0;
    while (1) {
        int conPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0);
        ret = XPR_RTSP_Open(conPort, urls[0]);
        XPR_ERR_ASSERT(ret);
        ret = XPR_RTSP_Close(conPort);
        XPR_ERR_ASSERT(ret);
        n++;
        if ((n % 1000) == 0)
            printf("Benchmark %u times\n", n);
    }
    XPR_RTSP_Close(mgrPort);
    XPR_RTSP_Fini();
}
