#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

const char* exampleFileName = "example.aac.framed";

void benchmark(void)
{
    XPR_RTSP_Init();
    int serverPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0);
    int streamPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 1, 0);
    int err = XPR_RTSP_Open(
        serverPort, "rtsp://0.0.0.0:8554/live/"
                    "0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
    XPR_ERR_ASSERT(err);
    err = XPR_RTSP_Start(serverPort);
    XPR_ERR_ASSERT(err);

    err = XPR_RTSP_Open(streamPort, "uri:///live-1?track=1&mime=audio/AAC");
    XPR_ERR_ASSERT(err);
    fprintf(stderr, "RTSP [%d] URL: %s\n", streamPort,
            "rtsp://0.0.0.0:8554/live-1");
    err = XPR_RTSP_Start(streamPort);
    XPR_ERR_ASSERT(err);

    // Open example file
    XPR_File* f1 = XPR_FileOpen(exampleFileName, "rb");
    if (XPR_FIEL_IS_NULL(f1)) {
        fprintf(stderr, "Open [%s] failed, errno: %d\n", exampleFileName,
                errno);
        abort();
    }

    XPR_StreamBlock stb;
    memset(&stb, 0, sizeof(stb));
    stb.bufferSize = 1024 * 1024;
    stb.buffer = malloc(stb.bufferSize);
    stb.data = stb.buffer;
    stb.dataSize = 0;
    stb.codec = AV_FOURCC_AAC;
    stb.track = 1;

    int64_t pts = 0;

    printf("!!! Fucked\n");

    while (1) {
        // Read frame size
        uint32_t frameSize = 0;
        int n = XPR_FileRead(f1, (uint8_t*)&frameSize, sizeof(frameSize));
        if (n != sizeof(frameSize)) {
            XPR_FileSeek(f1, 0, XPR_FILE_SEEK_SET);
            continue;
        }
        // Check frame size
        if (frameSize <= 0)
            continue;
        // Read frame content
        n = XPR_FileRead(f1, stb.data, frameSize);
        if (n != frameSize)
            continue;
        // Update stream block
        stb.dataSize = frameSize;
        stb.pts = pts;
        // Push to the server
        XPR_RTSP_PushData(streamPort, &stb);
        //
        XPR_ThreadSleep(38000);
        pts += 40000;
    }

    XPR_RTSP_Stop(streamPort);
    XPR_RTSP_Stop(serverPort);
    XPR_RTSP_Close(streamPort);
    XPR_RTSP_Close(serverPort);
    XPR_RTSP_Fini();
}
