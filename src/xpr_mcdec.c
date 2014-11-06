#include <stdio.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#else
#define _ALLOW_KEYWORD_MACROS
#define inline __inline
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
}
#endif
#include <xpr/xpr_mcdec.h>

#define XPR_MCDEC_CHANNEL_MIN  1
#define XPR_MCDEC_CHANNEL_MAX  16

typedef struct DecCtx {
    AVCodecContext* decCtx;
    AVFrame* decFrame;
    uint8_t* datas[8];
    int stride;
    int width;
    int height;
} DecCtx;

static DecCtx decCtxs[XPR_MCDEC_CHANNEL_MAX+1];

static int IsTargetValid(int target)
{
    if (target < XPR_MCDEC_CHANNEL_MIN || target > XPR_MCDEC_CHANNEL_MAX)
        return 0;
    return 1;
}

int XPR_MCDEC_Config(int option, const void* data, int length)
{
    return 0;
}

int XPR_MCDEC_Init(void)
{
    int i = XPR_MCDEC_CHANNEL_MIN;
    long errorCode = 0;
    AVCodec* codec = 0;
    //
    av_log_set_level(AV_LOG_QUIET);
    avcodec_register_all();
    //
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
        return -1;
    for (; i <= XPR_MCDEC_CHANNEL_MAX; i++) {
        decCtxs[i].decCtx = avcodec_alloc_context3(codec);
        decCtxs[i].decFrame = av_frame_alloc();
        //
        decCtxs[i].decCtx->thread_count = 1;
        avcodec_open2(decCtxs[i].decCtx, codec, NULL);
    }
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

int XPR_MCDEC_PushData(int target, const uint8_t* data, int length)
{
    int i = 0;
    int got = 0;
    AVPacket pkt = {0};
    AVCodecContext* ctx = 0;
    AVFrame* frame = 0;
    static FILE* fp = 0;
    //
    if (!IsTargetValid(target))
        return -1;
    ctx = decCtxs[target].decCtx;
    frame = decCtxs[target].decFrame;
    if (!ctx || !frame)
        return -1;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)data;
    pkt.size = length;
    pkt.dts = AV_NOPTS_VALUE;
    pkt.pts = AV_NOPTS_VALUE;
    avcodec_decode_video2(ctx, frame, &got, &pkt);
    if (got) {
        printf("got ========%d x %d, %lld, %d, %d\n",
               frame->width, frame->height, frame->pts,
               frame->interlaced_frame, frame->top_field_first);
        printf("%p = %d, %p = %d, %p = %d\n",
               frame->data[0], frame->linesize[0],
               frame->data[1], frame->linesize[1],
               frame->data[2], frame->linesize[2]);
#if 0
        if (!fp)
            fopen_s(&fp, "sample.yuv", "wb");
        for (i = 0; i<frame->height; i++)
            fwrite(frame->data[0]+i*frame->linesize[0], 1, frame->width, fp);
        for (i = 0; i<frame->height/2; i++)
            fwrite(frame->data[1]+i*frame->linesize[1], 1, frame->width/2, fp);
        for (i = 0; i<frame->height/2; i++)
            fwrite(frame->data[2]+i*frame->linesize[2], 1, frame->width/2, fp);
#endif
    }
    return 0;
}

int XPR_MCDEC_Reset(int target)
{
    return 0;
}
