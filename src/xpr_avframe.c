#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_avframe.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>

static int DestroyWithData(XPR_AVFrame* frame)
{
    XPR_Free(frame);
    return 0;
}

static int DestroyWithoutData(XPR_AVFrame* frame)
{
    XPR_Free(frame);
    return 0;
}

XPR_API XPR_AVFrame* XPR_AVFrameNew(void)
{
    XPR_AVFrame* frame = XPR_Alloc(sizeof(XPR_AVFrame));
    if (frame) {
        memset(frame, 0, sizeof(XPR_AVFrame));
        frame->release = DestroyWithoutData;
    }
    return frame;
}

XPR_API XPR_AVFrame* XPR_AVFrameNewVideo(int format, int width, int height)
{
    XPR_AVFrame* frame = NULL;
    size_t totalBytes = sizeof(XPR_AVFrame) + 8;
    switch (format) {
    case XPR_AV_PIX_FMT_YUV420P:
        totalBytes += width * height * 3 / 2;
        break;
    default:
        break;
    }
    frame = XPR_Alloc(totalBytes);
    if (frame) {
        memset(frame, 0, sizeof(XPR_AVFrame));
        frame->format = format;
        frame->width = width;
        frame->height = height;
        switch (format) {
        case XPR_AV_PIX_FMT_YUV420P:
            frame->pitches[0] = width;
            frame->pitches[1] = width / 2;
            frame->pitches[2] = width / 2;
            frame->datas[0] = ((uint8_t*)frame) + sizeof(XPR_AVFrame) + 8;
            frame->datas[1] = frame->datas[0] + frame->pitches[0] * height;
            frame->datas[2] = frame->datas[1] + frame->pitches[1] * height / 2;
            break;
        default:
            break;
        }
        frame->release = DestroyWithData;
    }
    return frame;
}

XPR_API int XPR_AVFrameRelease(XPR_AVFrame* frame)
{
    if (frame->release)
        frame->release(frame);
    return 0;
}

static void CopyFrameInfo(const XPR_AVFrame* src, XPR_AVFrame* dst)
{
    int i = 0;
    dst->channelLayout = src->channelLayout;
    dst->dts = src->dts;
    dst->flags = src->flags;
    dst->format = src->format;
    dst->height = src->height;
    dst->opaque = src->opaque;
    for (i = 0; i < XPR_AVFRAME_PLANES; i++)
        dst->pitches[i] = src->pitches[i];
    dst->pts = src->pts;
    dst->sampleRate = src->sampleRate;
    dst->samples = src->samples;
    dst->width = src->width;
}

XPR_API XPR_AVFrame* XPR_AVFrameClone(const XPR_AVFrame* src)
{
    XPR_AVFrame* frame = NULL;
    switch (src->format) {
    case XPR_AV_PIX_FMT_YUV420P:
        frame = XPR_AVFrameNewVideo(src->format, src->width, src->height);
        CopyFrameInfo(src, frame);
        memcpy(frame->datas[0], src->datas[0], src->pitches[0] * src->height);
        memcpy(frame->datas[1], src->datas[1],
               src->pitches[1] * src->height / 2);
        memcpy(frame->datas[2], src->datas[2],
               src->pitches[2] * src->height / 2);
        break;
    }
    return frame;
}

XPR_API int XPR_AVFrameCopy(const XPR_AVFrame* src, XPR_AVFrame* dst)
{
    int ret = XPR_ERR_OK;
    if (!src || !dst)
        return XPR_ERR_GEN_NULL_PTR;
    if (src->format != dst->format || src->width != dst->width ||
        src->height != dst->height)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    switch (src->format) {
    case XPR_AV_PIX_FMT_YUV420P:
        CopyFrameInfo(src, dst);
        memcpy(dst->datas[0], src->datas[0], src->pitches[0] * src->height);
        memcpy(dst->datas[1], src->datas[1], src->pitches[1] * src->height / 2);
        memcpy(dst->datas[2], src->datas[2], src->pitches[2] * src->height / 2);
        break;
    default:
        ret = XPR_ERR_GEN_NOT_SUPPORT;
        break;
    }
    return ret;
}