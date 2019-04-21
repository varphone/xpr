#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#define XPR_ERR_ASSERT(err)                                                    \
    if (err != XPR_ERR_OK) {                                                   \
        printf("%s:%d, XPR_ErrorCode = 0x%08X\n", __FILE__, __LINE__, err);    \
        abort();                                                               \
    }

const char* urls[] = {
    "rtsp://127.0.0.1:8554/example.264",
    "rtsp://127.0.0.1:8554/example.aac",
    "rtsp://127.0.0.1:8554/example.mp4",
    "rtsp://127.0.0.1:8558/example.mp4", // Unreached Port
    "rtsp://192.168.0.1:8554/example.mp4", // Unreached Private Address
    "rtsp://196.168.0.1:8554/example.mp4", // Unreached Public Address
};

const int numUrls = sizeof(urls) / sizeof(urls[0]);

void benchmark(void)
{
    XPR_RTSP_Init();
    int ret;
    int mgrPport = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    ret = XPR_RTSP_Open(mgrPport, "dummy://any");
    XPR_ERR_ASSERT(ret);
    ret =XPR_RTSP_Start(mgrPport);
    XPR_ERR_ASSERT(ret);
    uint32_t n = 0;
    while (1) {
        int conPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0);
        ret = XPR_RTSP_Open(conPort, urls[n % numUrls]);
        XPR_ERR_ASSERT(ret);
        ret = XPR_RTSP_Start(conPort);
        //XPR_ERR_ASSERT(ret);
        while ((ret = XPR_RTSP_Stop(conPort)) != XPR_ERR_OK) {
            //printf("XPR_RTSP_Stop() ret = %08X\n", ret);
            XPR_ThreadSleep(100000);
        }
        //XPR_ERR_ASSERT(ret);
        while ((ret = XPR_RTSP_Close(conPort)) != XPR_ERR_OK) {
            //printf("XPR_RTSP_Close() ret = %08X\n", ret);
            XPR_ThreadSleep(100000);
        }
        //ret = XPR_RTSP_Close(conPort);
        //XPR_ERR_ASSERT(ret);
        n++;
        if ((n % 1000) == 0)
            printf("Benchmark %u times\n", n);
        XPR_ThreadSleep(100000);
    }
    getchar();
    ret = XPR_RTSP_Close(mgrPport);
    XPR_ERR_ASSERT(ret);
    XPR_RTSP_Fini();
}
