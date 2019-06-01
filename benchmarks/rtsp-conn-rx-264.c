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

// NOTE:
// The contents after "??" is use to config the connection working params.
// - autoRestart: Enable auto restart when src stopped or timeouted
// - restartDelay: Set auto restart delay in us
// - rxTimeout: Set rtp receive timeout in us
// - useFrameMerger: Enable frame merger on 264/265 fragments
// - rtpOverTcp: Enable/Disable RTP transimition over TCP
#define CONN_CFG                                                               \
    "??"                                                                       \
    "autoRestart=true&"                                                        \
    "restartDelay=1000000&"                                                    \
    "rxTimeout=2000000&"                                                       \
    "useFrameMerger=true&"                                                     \
    "rtpOverTcp=true"

const char* urls[] = {
    "rtsp://127.0.0.1:8554/example.264",
    "rtsp://127.0.0.1:8554/example.264" CONN_CFG,
};

const int numUrls = sizeof(urls) / sizeof(urls[0]);

static int data_handler(void* opaque, int port, const XPR_StreamBlock* stb)
{
    printf("Port %x, codec %x, flags %x, size %d, pts %ld\n", port, stb->codec,
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
    int conPorts[2];
    conPorts[0] = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0);
    ret = XPR_RTSP_Open(conPorts[0], urls[0]);
    XPR_ERR_ASSERT(ret);
    XPR_RTSP_AddDataCallback(conPorts[0], data_handler, (void*)0);
    XPR_RTSP_AddEventCallback(conPorts[0], event_handler, (void*)0);
    XPR_RTSP_SetAuth(conPorts[0], "admin", "123456", 0);
    ret = XPR_RTSP_Start(conPorts[0]);
    XPR_ERR_ASSERT(ret);

    conPorts[1] = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 2, 0);
    ret = XPR_RTSP_Open(conPorts[1], urls[1]);
    XPR_ERR_ASSERT(ret);
    XPR_RTSP_AddDataCallback(conPorts[1], data_handler, (void*)0);
    XPR_RTSP_AddEventCallback(conPorts[1], event_handler, (void*)0);
    XPR_RTSP_SetAuth(conPorts[1], "admin", "123456", 0);
    ret = XPR_RTSP_Start(conPorts[1]);
    XPR_ERR_ASSERT(ret);

    printf("Press any key to stop\n");
    getchar();

    // Stop and close rtsp connection(s)
    ret = XPR_RTSP_Stop(conPorts[0]);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(conPorts[0]);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Stop(conPorts[1]);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(conPorts[1]);
    XPR_ERR_ASSERT(ret);

    // Stop and close rtsp manager
    ret = XPR_RTSP_Stop(mgrPort);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(mgrPort);
    XPR_ERR_ASSERT(ret);

    // Cleanup the XPR_RTSP module
    XPR_RTSP_Fini();
}
