#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

const char* exampleFileName = "example.264.framed";

int eventCallback(void* opaque, int port, const XPR_RTSP_EVD* evd)
{
    if (evd->event == XPR_RTSP_EVT_UAC) {
        XPR_RTSP_EVD_UAC const* evdUAC = (XPR_RTSP_EVD_UAC const*)evd->data;
        char hoststr[NI_MAXHOST];
        char portstr[NI_MAXSERV];
        socklen_t client_len = sizeof(struct sockaddr_storage);
        int rc = getnameinfo((struct sockaddr*)evdUAC->client_addr, client_len,
                             hoststr, sizeof(hoststr), portstr, sizeof(portstr),
                             NI_NUMERICHOST | NI_NUMERICSERV);
        printf("User \"%s\" from %s:%s access to \"%s\"\n", evdUAC->username,
               hoststr, portstr, evdUAC->url_suffix);
    }
    return XPR_ERR_OK;
}

void benchmark(void)
{
    XPR_RTSP_Init();
    // Open and start the server port first
    int serverPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0);
    int err = XPR_RTSP_Open(
        serverPort, "rtsp://admin:admin@0.0.0.0:8554/live/"
                    "0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
    XPR_ERR_ASSERT(err);
    err = XPR_RTSP_AddEventCallback(serverPort, eventCallback, NULL);
    XPR_ERR_ASSERT(err);
    err = XPR_RTSP_Start(serverPort);
    XPR_ERR_ASSERT(err);
    // Open and start stream port after server started
    int streamPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 1, 0);
    // Query:
    // - track - Index of the track, must be first param of each track
    // - mime - Track data mime type, like: audio/AAC, video/H264
    err =
        XPR_RTSP_Open(streamPort, "uri:///example.264"
                                  "?track=1&mime=video/H264&appendOriginPTS=1");
    XPR_ERR_ASSERT(err);
    fprintf(stderr, "RTSP [%d] URL: %s\n", streamPort,
            "rtsp://0.0.0.0:8554/example.264");
    err = XPR_RTSP_Start(streamPort);
    XPR_ERR_ASSERT(err);
    // Open example file
    XPR_File* f1 = XPR_FileOpen(exampleFileName, "rb");
    if (XPR_FIEL_IS_NULL(f1)) {
        fprintf(stderr, "Open [%s] failed, errno: %d\n", exampleFileName,
                errno);
        abort();
    }
    // Allocate stream block and set public fileds
    XPR_StreamBlock stb;
    memset(&stb, 0, sizeof(stb));
    stb.bufferSize = 1024 * 1024;
    stb.buffer = malloc(stb.bufferSize);
    stb.data = stb.buffer;
    stb.dataSize = 0;
    stb.codec = AV_FOURCC_H264;
    stb.track = 1;
    // Timestamp accumulator
    int64_t pts = 0;
    // Loop for read and send frame
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
        stb.dts = stb.pts = pts;
        // Push to the server
        XPR_RTSP_PushData(streamPort, &stb);
        // 40000 us per frame for 25 fps
        XPR_ThreadSleep(40000);
        pts += 40000;
    }
    // Stop and close stream port first
    XPR_RTSP_Stop(streamPort);
    XPR_RTSP_Close(streamPort);
    // Stop server port after all stream closed
    XPR_RTSP_Stop(serverPort);
    XPR_RTSP_Close(serverPort);
    XPR_RTSP_Fini();
}
