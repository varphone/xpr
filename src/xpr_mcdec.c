#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#else
#define _ALLOW_KEYWORD_MACROS
#define inline __inline
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_mcdec.h>
#include <xpr/xpr_utils.h>
#include "drivers/mcdec/libav/runtime.h"
#include "drivers/mcdec/libav/task.h"
#include "drivers/mcdec/libav/worker.h"

#define XPR_MCDEC_CHANNEL_MIN  1
#define XPR_MCDEC_CHANNEL_MAX  16

typedef struct HandlerContext {
    void* handler;
    void* opaque;
} HandlerContext;

typedef struct PortParams {
    int imageWidth;
    int imageHeight;
    int imageQuality;
} PortParams;

typedef struct PortContext {
    AVCodec* avcodec;
    AVCodecContext* avctx;
    AVFrame* avfrm;
    uint8_t* avpktBuf;
    int avpktBufSize;
    PortParams params;
} PortContext;

typedef struct DecCtx {
    AVCodecContext* decCtx;
    AVFrame* decFrame;
    uint8_t* datas[8];
    int stride;
    int width;
    int height;
} DecCtx;

static HandlerContext xpr_mcdec_avfrm_handlers[XPR_MCDEC_MAX_HANDLERS];
static HandlerContext xpr_mcdec_stblk_handlers[XPR_MCDEC_MAX_HANDLERS];
static PortContext xpr_mcdec_ports[2][16] = {0};

static XPR_MCDEC_AVFrameHandler avFrameHandlers[16];
static void* avFrameHandlerOpaques[16];

static DecCtx decCtxs[XPR_MCDEC_CHANNEL_MAX+1];

static int IsPortValid(int port)
{
    int major = XPR_MCDEC_PORT_MAJOR(port);
    int minor = XPR_MCDEC_PORT_MINOR(port);
    if (minor < XPR_MCDEC_CHANNEL_MIN || minor > XPR_MCDEC_CHANNEL_MAX)
        return XPR_FALSE;
    return XPR_TRUE;
}

int XPR_MCDEC_Config(int option, const void* data, int length)
{
    return 0;
}

static void InitSpecialPorts(void)
{
    AVCodec* codec = 0;
    PortContext* pc = 0;
    // Init bmp image converter
    pc = &xpr_mcdec_ports[0][XPR_MCDEC_PORT_MINOR(XPR_MCDEC_SPEC_BMPENC_PORT)];
    pc->avcodec = avcodec_find_encoder(AV_CODEC_ID_BMP);
    if (pc->avcodec) {
        pc->avctx = avcodec_alloc_context3(pc->avcodec);
        pc->avctx->bit_rate = 0;
        pc->avctx->time_base.den = 1;
        pc->avctx->time_base.num = 1;
        pc->avctx->pix_fmt = AV_PIX_FMT_BGR24;
        pc->avctx->width = 0;
        pc->avctx->height = 0;
        pc->avctx->qmin = 1;
        pc->avctx->qmax = 100;
        pc->avctx->thread_count = 0;
        pc->avctx->thread_type = 0;
        pc->avpktBufSize = 8000000;
        pc->avpktBuf = (uint8_t*)XPR_Alloc(pc->avpktBufSize);
        pc->params.imageQuality = 100;
    }
    // Init jpeg image converter
    pc = &xpr_mcdec_ports[0][XPR_MCDEC_PORT_MINOR(XPR_MCDEC_SPEC_JPGENC_PORT)];
    pc->avcodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (pc->avcodec) {
        pc->avctx = avcodec_alloc_context3(pc->avcodec);
        pc->avctx->bit_rate = 0;
        pc->avctx->time_base.den = 1;
        pc->avctx->time_base.num = 1;
        pc->avctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
        pc->avctx->width = 0;
        pc->avctx->height = 0;
        pc->avctx->qmin = 1;
        pc->avctx->qmax = 100;
        pc->avctx->thread_count = 0;
        pc->avctx->thread_type = 0;
        pc->avpktBufSize = 1000000;
        pc->avpktBuf = (uint8_t*)XPR_Alloc(pc->avpktBufSize);
        pc->params.imageQuality = 100;
    }
    // Init png image converter
    pc = &xpr_mcdec_ports[0][XPR_MCDEC_PORT_MINOR(XPR_MCDEC_SPEC_PNGENC_PORT)];
    pc->avcodec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    if (pc->avcodec) {
        pc->avctx = avcodec_alloc_context3(pc->avcodec);
        pc->avctx->bit_rate = 0;
        pc->avctx->time_base.den = 1;
        pc->avctx->time_base.num = 1;
        pc->avctx->pix_fmt = AV_PIX_FMT_YUVJ420P;
        pc->avctx->width = 0;
        pc->avctx->height = 0;
        pc->avctx->qmin = 1;
        pc->avctx->qmax = 100;
        pc->avctx->thread_count = 0;
        pc->avctx->thread_type = 0;
        pc->avpktBufSize = 1000000;
        pc->avpktBuf = (uint8_t*)XPR_Alloc(pc->avpktBufSize);
        pc->params.imageQuality = 80;
    }
}

int XPR_MCDEC_Init(void)
{
    int i = XPR_MCDEC_CHANNEL_MIN;
    long errorCode = 0;
    AVCodec* codec = 0;
    PortContext* pc = 0;
    //
    memset(&avFrameHandlers, 0, sizeof(avFrameHandlers));
    memset(&avFrameHandlerOpaques, 0, sizeof(avFrameHandlerOpaques));
    av_log_set_level(AV_LOG_DEBUG);
    avcodec_register_all();
    //
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
        return -1;
    for (; i <= XPR_MCDEC_CHANNEL_MAX; i++) {
        decCtxs[i].decCtx = avcodec_alloc_context3(codec);
        decCtxs[i].decFrame = av_frame_alloc();
        //
        decCtxs[i].decCtx->delay = 0;
        //decCtxs[i].decCtx->skip_frame = AVDISCARD_NONKEY;
        decCtxs[i].decCtx->thread_count = 0;
        decCtxs[i].decCtx->thread_type = 0;
        avcodec_open2(decCtxs[i].decCtx, codec, NULL);
    }
    //
    InitSpecialPorts();
    //
    return 0;
}

int XPR_MCDEC_Fini(void)
{
    int i = XPR_MCDEC_CHANNEL_MIN;
    for (; i <= XPR_MCDEC_CHANNEL_MAX; i++) {
        if (decCtxs[i].decCtx) {
            avcodec_close(decCtxs[i].decCtx);
            av_frame_free(&decCtxs[i].decFrame);
            decCtxs[i].decCtx = 0;
        }
    }
    return 0;
}

static uint32_t fourccMap[] = {
    AV_FOURCC_NULL,
    AV_FOURCC_BMPI,
    AV_FOURCC_JPGI,
    AV_FOURCC_PNGI,
};

static int AVFrameToImage(int port, const XPR_AVFrame* frm)
{
    int got = 0;
    int scaled = 0;
    int force_scaled = 0;
    int dst_pix_fmt = 0;
    AVFrame avfrm;
    AVPacket avpkt;
    AVPicture avpic;
    XPR_StreamBlock stblk;
    struct SwsContext* swsctx = 0;
    PortContext* pc = &xpr_mcdec_ports[0][XPR_MCDEC_PORT_MINOR(port)];
    //
    memset(&avfrm, 0, sizeof(avfrm));
    if (pc->avctx) {
        avfrm.data[0] = frm->datas[0];
        avfrm.data[1] = frm->datas[1];
        avfrm.data[2] = frm->datas[2];
        avfrm.linesize[0] = frm->pitches[0];
        avfrm.linesize[1] = frm->pitches[1];
        avfrm.linesize[2] = frm->pitches[2];
        avfrm.extended_data = (uint8_t**)frm->datas;
        avfrm.width = frm->width;
        avfrm.height = frm->height;
        avfrm.format = frm->format;
        avfrm.pts = AV_NOPTS_VALUE;
        //
        av_init_packet(&avpkt);
        avpkt.data = pc->avpktBuf;
        avpkt.size = pc->avpktBufSize;
        //
        pc->avctx->qmin = MIN(100, MAX(1, 100 - pc->params.imageQuality));
        //
        if (port == XPR_MCDEC_SPEC_BMPENC_PORT && frm->format != pc->avctx->pix_fmt)
            force_scaled =1;

        if ((pc->params.imageWidth == 0 || pc->params.imageHeight == 0) && !force_scaled) {
            pc->avctx->width = frm->width;
            pc->avctx->height = frm->height;
        }
        else {
            pc->avctx->width = pc->params.imageWidth ? pc->params.imageWidth : frm->width;
            pc->avctx->height = pc->params.imageHeight ? pc->params.imageHeight : frm->height;
            dst_pix_fmt = port == XPR_MCDEC_SPEC_BMPENC_PORT ? AV_PIX_FMT_BGR24 : avfrm.format;
            swsctx = sws_getContext(avfrm.width, avfrm.height, (enum AVPixelFormat)avfrm.format, 
                                    pc->avctx->width, pc->avctx->height, (enum AVPixelFormat)dst_pix_fmt,
                                    SWS_BICUBIC|SWS_CPU_CAPS_MMX|SWS_CPU_CAPS_SSE2, NULL, NULL, NULL);
            if (swsctx) {
                if (avpicture_alloc(&avpic, (enum AVPixelFormat)dst_pix_fmt, pc->avctx->width, pc->avctx->height) == 0) {
                    if (sws_scale(swsctx, avfrm.data, avfrm.linesize, 0, avfrm.height, avpic.data, avpic.linesize) > 0)
                        scaled = 1;
                    else
                        avpicture_free(&avpic);
                    sws_freeContext(swsctx);
                    //
                    avfrm.data[0] = avpic.data[0];
                    avfrm.data[1] = avpic.data[1];
                    avfrm.data[2] = avpic.data[2];
                    avfrm.linesize[0] = avpic.linesize[0];
                    avfrm.linesize[1] = avpic.linesize[1];
                    avfrm.linesize[2] = avpic.linesize[2];
                    avfrm.extended_data = (uint8_t**)avpic.data;
                    avfrm.width = pc->avctx->width;
                    avfrm.height = pc->avctx->height;
                    avfrm.format = dst_pix_fmt;
                }
            }
        }
        if (avcodec_open2(pc->avctx, pc->avcodec, NULL) < 0) {
            if (scaled)
                avpicture_free(&avpic);
            return XPR_ERR_ERROR;
        }
        //
        avcodec_encode_video2(pc->avctx, &avpkt, &avfrm, &got);
        avcodec_close(pc->avctx);
        if (scaled)
            avpicture_free(&avpic);
        if (got) {
            stblk.buffer = stblk.data = avpkt.data;
            stblk.bufferSize = stblk.dataSize = avpkt.size;
            stblk.codec = fourccMap[XPR_MCDEC_PORT_MINOR(port)];
            stblk.dts = stblk.pts = frm->pts;
            XPR_MCDEC_DeliverStreamBlock(port, &stblk);
        }
    }
    return XPR_ERR_SUCCESS;
}

int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* frm)
{
    int result = XPR_ERR_ERROR;
    if (!IsPortValid(port))
        return XPR_ERR_ERROR;
    return AVFrameToImage(port, frm);
}

int XPR_MCDEC_PushData(int port, const uint8_t* data, int length)
{
    int got = 0;
    AVPacket pkt = {0};
    AVCodecContext* ctx = 0;
    AVFrame* frame = 0;
    XPR_AVFrame outFrame = {0};
    //
    if (!IsPortValid(port))
        return -1;
    ctx = decCtxs[XPR_MCDEC_PORT_MINOR(port)].decCtx;
    frame = decCtxs[XPR_MCDEC_PORT_MINOR(port)].decFrame;
    if (!ctx || !frame)
        return -1;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)data;
    pkt.size = length;
    pkt.dts = AV_NOPTS_VALUE;
    pkt.pts = AV_NOPTS_VALUE;
    avcodec_decode_video2(ctx, frame, &got, &pkt);
    if (got) {
        outFrame.datas[0] = frame->data[0];
        outFrame.datas[1] = frame->data[2];
        outFrame.datas[2] = frame->data[1];
        outFrame.pitches[0] = frame->linesize[0];
        outFrame.pitches[1] = frame->linesize[2];
        outFrame.pitches[2] = frame->linesize[1];
        outFrame.format = XPR_AV_PIX_FMT_YUV420P;
        outFrame.width = frame->width;
        outFrame.height = frame->height;
        XPR_MCDEC_DeliverAVFrame(port, &outFrame);
    }
    return XPR_ERR_SUCCESS;
}

int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* blk)
{
    int i = 0;
    int got = 0;
    AVPacket pkt = {0};
    AVCodecContext* ctx = 0;
    AVFrame* frame = 0;
    XPR_AVFrame outFrame;
    static FILE* fp = 0;
    //
    if (!IsPortValid(port))
        return -1;
    ctx = decCtxs[XPR_MCDEC_PORT_MINOR(port)].decCtx;
    frame = decCtxs[XPR_MCDEC_PORT_MINOR(port)].decFrame;
    if (!ctx || !frame)
        return -1;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)XPR_StreamBlockData(blk);
    pkt.size = XPR_StreamBlockLength(blk);
    pkt.dts = AV_NOPTS_VALUE;
    pkt.pts = AV_NOPTS_VALUE;
    avcodec_decode_video2(ctx, frame, &got, &pkt);
    if (got) {
        outFrame.datas[0] = frame->data[0];
        outFrame.datas[1] = frame->data[2];
        outFrame.datas[2] = frame->data[1];
        outFrame.pitches[0] = frame->linesize[0];
        outFrame.pitches[1] = frame->linesize[2];
        outFrame.pitches[2] = frame->linesize[1];
        outFrame.format = XPR_AV_PIX_FMT_YUV420P;
        outFrame.width = frame->width;
        outFrame.height = frame->height;
        XPR_MCDEC_DeliverAVFrame(port, &outFrame);
    }
    return 0;
}

int XPR_MCDEC_Reset(int target)
{
    return XPR_ERR_SUCCESS;
}

void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avfrm)
{
    int i = 0;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (xpr_mcdec_avfrm_handlers[i].handler) {
            ((XPR_MCDEC_AVFrameHandler)xpr_mcdec_avfrm_handlers[i].handler)(
                    xpr_mcdec_avfrm_handlers[i].opaque, port, avfrm);
        }
    }
}

void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* blk)
{
    int i = 0;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (xpr_mcdec_stblk_handlers[i].handler) {
            ((XPR_MCDEC_StreamBlockHandler)xpr_mcdec_stblk_handlers[i].handler)(
                    xpr_mcdec_stblk_handlers[i].opaque, port, blk);
        }
    }
}

int XPR_MCDEC_AddAVFrameHandler(XPR_MCDEC_AVFrameHandler handler, void* opaque)
{
    int i = 0;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (xpr_mcdec_avfrm_handlers[i].handler == 0) {
            xpr_mcdec_avfrm_handlers[i].handler = handler;
            xpr_mcdec_avfrm_handlers[i].opaque = opaque;
            return XPR_ERR_SUCCESS;
        }
    }
    return XPR_ERR_ERROR;
}

int XPR_MCDEC_AddStreamBlockHandler(XPR_MCDEC_StreamBlockHandler handler, void* opaque)
{
    int i = 0;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (xpr_mcdec_stblk_handlers[i].handler == 0) {
            xpr_mcdec_stblk_handlers[i].handler = handler;
            xpr_mcdec_stblk_handlers[i].opaque = opaque;
            return XPR_ERR_SUCCESS;
        }
    }
    return XPR_ERR_ERROR;
}

int XPR_MCDEC_SetParam(int port, int param, const void* data, int size)
{
    PortContext* pc = 0;
    if (!IsPortValid(port))
        return XPR_ERR_ERROR;
    pc = &xpr_mcdec_ports[XPR_MCDEC_PORT_MAJOR(port)][XPR_MCDEC_PORT_MINOR(port)];
    switch (param) {
    case XPR_MCDEC_PARAM_IMAGE_WIDTH:
        pc->params.imageWidth = size ? *(int*)data : (int)data;
        break;
    case XPR_MCDEC_PARAM_IMAGE_HEIGHT:
        pc->params.imageHeight = size ? *(int*)data : (int)data;
        break;
    case XPR_MCDEC_PARAM_IMAGE_QUALITY:
        pc->params.imageQuality = size ? *(int*)data : (int)data;
        break;
    }
    return XPR_ERR_SUCCESS;
}

int XPR_MCDEC_GetParam(int port, int param, void* data, int* size)
{
    return XPR_ERR_SUCCESS;
}
