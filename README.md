X Portable Runtime {#mainpage}
==================

## Compoments

* [AVFRAME](#avframe)
* [BASE64](#base64)
* [DPU](#dpu)
* [FIFO](#fifo)
* [JSON](#json)
* [MD5](#md5)
* [PLUGIN](#plugin)
* [RTSP](#rtsp)
* [SYNC](#sync)
* [SYS](#sys)
* [THREAD](#thread)
* [UPS](#ups)
* [URL](#url)
* [UTILS](#utils)

## AVFRAME

Audio and Video raw data container

## BASE64

BASE64 algorithm

## DPU

Data Processing Unit

## FIFO

Fifo

## JSON

Java Script Object Notiation

## MD5

MD5 algorithm

## PLUGIN

Plugin framework

## RTSP

### RTSP 服务器

**使用示例**
```
const char* url = "rtsp://0.0.0.0:554?maxStreams=16&maxStreamTracks=4&maxWorkers=1&workerDelay=10"
int port = XPR_RTSP_Open(XPR_RTSP_PORT(2, 0, 0), url);
if (port > 0) {
    XPR_RTSP_Start(port);

    int ch1 = XPR_RTSP_Open(XPR_RTSP_PORT(2, 1, 0), "uri:///channel/1?tracks=video/H264;audio/G711");
    if (ch1 > 0) {
        XPR_RTSP_Start(ch1);
    }
    ...
    XPR_RTSP_Stop(port);
    XPR_RTSP_Close(port);
}
```

## SYNC

Thread and Process synchronization

## SYS

System relates

## THREAD

Thread

## UPS

Universal Preference Settings framework

## URL

Url parser

## UTILS

Utilities

