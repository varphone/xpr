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

const char* exampleFileName = "example.264.framed";
static char logBuffer[1024];
static char logHeader[] = " Index     Offset Length Header\n";

void benchmark(void)
{
    XPR_RTSP_Init();
    // Open and start the server port first
    int serverPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0);
    int err = XPR_RTSP_Open(
        serverPort, "rtsp://0.0.0.0:8554/live/"
                    "0?maxStreams=16&maxStreamTracks=4&maxWorkers=1&high");
    XPR_ERR_ASSERT(err);
    err = XPR_RTSP_Start(serverPort);
    XPR_ERR_ASSERT(err);
    // Open and start stream port after server started
    int streamPort = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 1, 0);
    // Query:
    // - track - Index of the track, must be first param of each track
    // - mime - Track data mime type, like: audio/AAC, video/H264
    err = XPR_RTSP_Open(streamPort, "uri:///live/1"
                                    "?track=1&mime=video/H264");
    XPR_ERR_ASSERT(err);
    fprintf(stderr, "RTSP [%d] URL: %s\n", streamPort,
            "rtsp://0.0.0.0:8554/live/1");
    err = XPR_RTSP_Start(streamPort);
    XPR_ERR_ASSERT(err);
    // Open example file
    XPR_File* f1 = XPR_FileOpen(exampleFileName, "rb");
    if (XPR_FIEL_IS_NULL(f1)) {
        fprintf(stderr, "Open [%s] failed, errno: %d\n", exampleFileName,
                errno);
        abort();
    }
    // Prepare logging files
    XPR_File* dataFile = XPR_FileOpen("rtsp-svr-tx-264-with-log.264", "cwb");
    XPR_File* logFile = XPR_FileOpen("rtsp-svr-tx-264-with-log.txt", "cwb");
    int n = sprintf(logBuffer, "%s", logHeader);
    XPR_FileWrite(logFile, logBuffer, n);
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
    // Logging accumulators
    uint32_t frames = 0;
    uint32_t offset = 0;
    printf("Press any key to start ...\n");
    getchar();
    // Loop for read and send frame
    while (1) {
        // Read frame size
        uint32_t frameSize = 0;
        int n = XPR_FileRead(f1, (uint8_t*)&frameSize, sizeof(frameSize));
        if (n != sizeof(frameSize)) {
            break;
        }
        // Check frame size
        if (frameSize <= 0)
            break;
        // Read frame content
        n = XPR_FileRead(f1, stb.data, frameSize);
        if (n != frameSize)
            break;
        // Update stream block
        stb.dataSize = frameSize;
        stb.dts = stb.pts = pts;
        // Push to the server
        XPR_RTSP_PushData(streamPort, &stb);
        // Logging
        XPR_FileWrite(dataFile, stb.data, stb.dataSize);
        n = sprintf(logBuffer, "%6d %10d %6d [%02X %02X %02X %02X %02X %02X]\n",
                    frames, offset, stb.dataSize, stb.data[0], stb.data[1],
                    stb.data[2], stb.data[3], stb.data[4], stb.data[5]);
        XPR_FileWrite(logFile, logBuffer, n);
        frames += 1;
        offset += stb.dataSize;
        // 40000 us per frame for 25 fps
        XPR_ThreadSleep(40000);
        pts += 40000;
    }
    // Flush to disk
    XPR_FileFlush(dataFile);
    XPR_FileClose(dataFile);
    XPR_FileFlush(logFile);
    XPR_FileClose(logFile);
    // Stop and close stream port first
    XPR_RTSP_Stop(streamPort);
    XPR_RTSP_Close(streamPort);
    // Stop server port after all stream closed
    XPR_RTSP_Stop(serverPort);
    XPR_RTSP_Close(serverPort);
    XPR_RTSP_Fini();
}
