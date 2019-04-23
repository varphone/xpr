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
    // - audioProfile - Profile level id, usually 1
    // - audioFreqIdx - Index of sampling frequency, 3=48000Hz 4=44100Hz 11=8000Hz
    // - audioChannles - Number of the channels, 1 or 2
    // - mime - Track data mime type, like: audio/AAC, video/H264
    err = XPR_RTSP_Open(streamPort, "uri:///live/1"
                                    "?track=1&audioProfile=1&audioFreqIdx=11&"
                                    "audioChannles=1&mime=audio/AAC");
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
    // Allocate stream block and set public fileds
    XPR_StreamBlock stb;
    memset(&stb, 0, sizeof(stb));
    stb.bufferSize = 4 * 1024;
    stb.buffer = malloc(stb.bufferSize);
    stb.data = stb.buffer;
    stb.dataSize = 0;
    stb.codec = AV_FOURCC_AAC;
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
        // 128 us per frame for 8000/16/1 AAC
        XPR_ThreadSleep(128000);
        pts += 128000;
    }
    // Stop and close stream port first
    XPR_RTSP_Stop(streamPort);
    XPR_RTSP_Close(streamPort);
    // Stop server port after all stream closed
    XPR_RTSP_Stop(serverPort);
    XPR_RTSP_Close(serverPort);
    XPR_RTSP_Fini();
}
