﻿#include <errno.h>
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

static int copyNalu(uint8_t* src, int srcSize, uint8_t* dst, int dstSize)
{
    uint32_t startCode = 0;
    uint32_t startCodeShift = 0;
    uint8_t* srcEnd = src + srcSize;
    uint8_t* dstOrig = dst;
    while (src < srcEnd) {
        uint32_t c = *src;
        if (c == 0x00 || c == 0x01) {
            startCode |= (c << startCodeShift);
            startCodeShift += 8;
            if (startCode == 0x01000000)
                break;
        }
        else {
            startCode = 0;
            startCodeShift = 0;
        }
        *dst = c;
        src++;
        dst++;
    }
    int n = dst - dstOrig;
    if (startCode == 0x01000000)
        n -= 3;
    return n;
}

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
                                    "?aql=15&vql=15&track=1&mime=video/H264");
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
    stb.bufferSize = 1024 * 1024;
    stb.buffer = malloc(stb.bufferSize);
    stb.data = stb.buffer;
    stb.dataSize = 0;
    stb.codec = AV_FOURCC_H264;
    stb.track = 1;
    // Timestamp accumulator
    int64_t pts = 0;
    int delta = 0;
    uint8_t* nbuf = malloc(stb.bufferSize);
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
#if 0
        uint8_t* ptr = stb.data + 4;
        uint32_t nl = frameSize - 4;
        //printf("frameSize = %d\n", frameSize);
        while (1) {
            int n = copyNalu(ptr, nl, nbuf, stb.bufferSize);
            //printf("N = %d\n", n);
            if (n <= 0)
                break;
        // Update stream block
        stb.data = nbuf;
        stb.dataSize = n;
        stb.dts = stb.pts = pts;
        stb.flags |= XPR_STREAMBLOCK_FLAG_FRAGMENTS;
        // Push to the server
        XPR_RTSP_PushData(streamPort, &stb);
            ptr += n + 4;
            nl -= n + 4;
        }
#else
        // Update stream block
        stb.dataSize = frameSize;
        stb.dts = stb.pts = pts;
        // Push to the server
        XPR_RTSP_PushData(streamPort, &stb);
#endif
        int64_t deltaPts = 20000;// + (delta++ % 200) * 100;
        // 40000 us per frame for 25 fps
        XPR_ThreadSleep(deltaPts);
        pts += deltaPts;
        //printf("deltaPts = %ld\n", deltaPts);
    }
    // Stop and close stream port first
    XPR_RTSP_Stop(streamPort);
    XPR_RTSP_Close(streamPort);
    // Stop server port after all stream closed
    XPR_RTSP_Stop(serverPort);
    XPR_RTSP_Close(serverPort);
    XPR_RTSP_Fini();
}
