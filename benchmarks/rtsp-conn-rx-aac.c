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
    "rtsp://127.0.0.1:8554/live/1",
};

const int numUrls = sizeof(urls) / sizeof(urls[0]);

static int data_handler(void* opaque, int port, const XPR_StreamBlock* stb)
{
    printf("Port %x, codec %x, flags %x, size %d, pts %lld\n", port, stb->codec,
           stb->flags, stb->dataSize, stb->pts);
    return 0;
}

static int event_handler(void* opaque, int port, const XPR_RTSP_EVD* evd)
{
    if (evd) {
        printf("Event: %d, ", evd->event);
        if (evd->event == XPR_RTSP_EVT_TIMEOUT) {
            printf("XPR_RTSP_EVT_TIMEOUT");
        }
        printf("\n");
    }
    return 0;
}

void benchmark(void)
{
    int ret;

    // Initialize the XPR_RTSP module
    XPR_RTSP_Init();

    // Open and start the rtsp manager
    int mgrPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    ret = XPR_RTSP_Open(mgrPort, "dummy://any");
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Start(mgrPort);
    XPR_ERR_ASSERT(ret);

    // Open and start the rtsp connection
    int conPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0);
    ret = XPR_RTSP_Open(conPort, urls[0]);
    XPR_ERR_ASSERT(ret);
    XPR_RTSP_AddDataCallback(conPort, data_handler, (void*)0);
    XPR_RTSP_AddEventCallback(conPort, event_handler, (void*)0);
    XPR_RTSP_SetAuth(conPort, "admin", "123456", 0);
    ret = XPR_RTSP_Start(conPort);
    XPR_ERR_ASSERT(ret);

    printf("Press any key to stop\n");
    getchar();

    // Stop and close rtsp connection(s)
    ret = XPR_RTSP_Stop(conPort);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(conPort);
    XPR_ERR_ASSERT(ret);

    // Stop and close rtsp manager
    ret = XPR_RTSP_Stop(mgrPort);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(mgrPort);
    XPR_ERR_ASSERT(ret);

    // Cleanup the XPR_RTSP module
    XPR_RTSP_Fini();
}
