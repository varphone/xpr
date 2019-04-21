#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_utils.h>

void benchmark(void)
{
    XPR_RTSP_Init();
    int port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    uint32_t n = 0;
    while (1) {
        if (XPR_RTSP_Open(port, "dummy://any") == XPR_ERR_OK) {
            XPR_RTSP_Close(port);
        }
        n++;
        if ((n % 1000) == 0)
            printf("Benchmark %u times\n", n);
    }
    XPR_RTSP_Fini();
}
