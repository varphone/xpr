#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_utils.h>

void benchmark(void)
{
    XPR_RTSP_Init();
    int ret;
    int port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    uint32_t n = 0;
    ret = XPR_RTSP_Open(port, "dummy://any");
    if (ret == XPR_ERR_OK) {
        while (1) {
            ret = XPR_RTSP_Start(port);
            if (ret == XPR_ERR_OK) {
                ret = XPR_RTSP_Stop(port);
            }
            if (ret == XPR_ERR_OK)
                n++;
            if ((n % 1000) == 0)
                printf("Benchmark %u times\n", n);
        }
    }
    XPR_RTSP_Close(port);
    XPR_RTSP_Fini();
}