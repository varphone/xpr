#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>
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
    "autoRestart=false&"                                                       \
    "restartDelay=1000000&"                                                    \
    "rxTimeout=10000000&"                                                      \
    "useFrameMerger=true&"                                                     \
    "rtpOverTcp=true"

const char* urls[] = {
    "rtsp://127.0.0.1:8554/live/1" CONN_CFG,
};

const int numUrls = sizeof(urls) / sizeof(urls[0]);

struct Context {
    int port;
    uint32_t frames;
    uint32_t offset;
    XPR_File* dataFile;
    XPR_File* logFile;
};

static char logBuffer[1024];
static char logHeader[] = " Index     Offset Length Header\n";

static int data_handler(void* opaque, int port, const XPR_StreamBlock* stb)
{
    int n = 0;
    struct Context* ctx = (struct Context*)opaque;
    if (ctx && stb->codec == AV_FOURCC_H264) {
        XPR_FileWrite(ctx->dataFile, stb->data, stb->dataSize);
        n = sprintf(logBuffer, "%6d %10d %6d [%02X %02X %02X %02X %02X %02X]\n",
                    ctx->frames, ctx->offset, stb->dataSize, stb->data[0],
                    stb->data[1], stb->data[2], stb->data[3], stb->data[4],
                    stb->data[5]);
        XPR_FileWrite(ctx->logFile, logBuffer, n);
        ctx->frames += 1;
        ctx->offset += stb->dataSize;
    }
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
    struct Context ctx = {-1, 0, 0, NULL, NULL};

    ctx.dataFile = XPR_FileOpen("rtsp-conn-rx-264-with-log.264", "cwb");
    ctx.logFile = XPR_FileOpen("rtsp-conn-rx-264-with-log.txt", "cwb");
    ret = sprintf(logBuffer, "%s", logHeader);
    XPR_FileWrite(ctx.logFile, logBuffer, ret);

    // Initialize the XPR_RTSP module
    XPR_RTSP_Init();

    // Open and start the rtsp manager
    int mgrPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 0, 0);
    ret = XPR_RTSP_Open(mgrPort, "dummy://any");
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Start(mgrPort);
    XPR_ERR_ASSERT(ret);

    // Open and start the rtsp connection
    ctx.port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0);
    ret = XPR_RTSP_Open(ctx.port , urls[0]);
    XPR_ERR_ASSERT(ret);
    XPR_RTSP_AddDataCallback(ctx.port , data_handler, (void*)&ctx);
    XPR_RTSP_AddEventCallback(ctx.port, event_handler, (void*)&ctx);
    XPR_RTSP_SetAuth(ctx.port , "admin", "123456", 0);
    ret = XPR_RTSP_Start(ctx.port);
    XPR_ERR_ASSERT(ret);

    printf("Press any key to stop ...\n");
    getchar();

    XPR_FileFlush(ctx.dataFile);
    XPR_FileFlush(ctx.logFile);

    // Stop and close rtsp connection(s)
    ret = XPR_RTSP_Stop(ctx.port);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(ctx.port);
    XPR_ERR_ASSERT(ret);

    // Stop and close rtsp manager
    ret = XPR_RTSP_Stop(mgrPort);
    XPR_ERR_ASSERT(ret);
    ret = XPR_RTSP_Close(mgrPort);
    XPR_ERR_ASSERT(ret);

    // Cleanup the XPR_RTSP module
    XPR_RTSP_Fini();

    XPR_FileClose(ctx.dataFile);
    XPR_FileClose(ctx.logFile);
}
