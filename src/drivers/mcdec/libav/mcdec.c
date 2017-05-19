#ifdef HAVE_XPR_MCDEC_DRIVER_LIBAV

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif
#ifdef __cplusplus
extern "C" {
#else
#define _ALLOW_KEYWORD_MACROS
#define inline __inline
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_mcdec.h>
#include <xpr/xpr_utils.h>

#define XPR_MCDEC_CHANNEL_MIN  1
#define XPR_MCDEC_CHANNEL_MAX  16

typedef struct Configuration {
	int c4wga;
	int log_level;
} Configuration;

typedef struct HandlerContext {
    void* handler;
    void* opaque;
} HandlerContext;

typedef struct PortParams {
	int block;
	int deinterlace;
	uint32_t force_fourcc;
    int image_width;
    int image_height;
    int image_quality;
	int skip_frame;
} PortParams;

typedef struct PortContext {
	uint32_t flags;
    AVCodec* avcodec;
    AVCodecContext* avctx;
    AVFrame* avfrm;
    uint8_t* avpkt_buf;
    int avpkt_buf_size;
    PortParams params;
	uint32_t cur_fourcc;
	XPR_AVFrame out_frame;
	uint8_t* out_frame_buffer;
	HandlerContext out_frame_handlers[XPR_MCDEC_MAX_HANDLERS];
	HandlerContext out_stblk_handlers[XPR_MCDEC_MAX_HANDLERS];
} PortContext;

static HandlerContext xpr_mcdec_avfrm_handlers[XPR_MCDEC_MAX_HANDLERS];
static HandlerContext xpr_mcdec_stblk_handlers[XPR_MCDEC_MAX_HANDLERS];
static Configuration xpr_mcdec_cfg = { 0, 0 };
static PortContext xpr_mcdec_ports[2][16];

static PortContext* GetPortContext(int port)
{
	return &xpr_mcdec_ports[XPR_MCDEC_PORT_MAJOR(port)][XPR_MCDEC_PORT_MINOR(port)];
}

static int FlushPort(int port)
{
	PortContext* pc = GetPortContext(port);
	if (pc) {
		if (pc->avctx) {
			avcodec_flush_buffers(pc->avctx);
		}
		pc->flags |= XPR_MCDEC_FLAG_SYNRQ;
		fprintf(stderr, "XPR_MCDEC: Port [%x] flushed\n", port);
	}
	return XPR_ERR_OK;
}

static int IsPortBlocked(int port)
{
	PortContext* pc = GetPortContext(port);
	return pc->flags & XPR_MCDEC_FLAG_BLKED ? XPR_TRUE : XPR_FALSE;
}

static int IsPortClosed(int port)
{
	PortContext* pc = GetPortContext(port);
	return pc->flags & XPR_MCDEC_FLAG_CLOSE ? XPR_TRUE : XPR_FALSE;
}

static int IsPortValid(int port)
{
    int major = XPR_MCDEC_PORT_MAJOR(port);
    int minor = XPR_MCDEC_PORT_MINOR(port);
    if (minor < XPR_MCDEC_CHANNEL_MIN || minor > XPR_MCDEC_CHANNEL_MAX)
        return XPR_FALSE;
    return XPR_TRUE;
}

static int IsPortReady(int port, uint32_t fourcc)
{
	PortContext* pc = GetPortContext(port);
	return (pc->cur_fourcc != 0 &&
			pc->cur_fourcc != AV_FOURCC_NULL &&
			pc->cur_fourcc == fourcc) ? XPR_TRUE : XPR_FALSE;
}

static void SetupSkipFrame(PortContext* pc)
{
	switch (pc->params.skip_frame) {
	case 0:
		pc->avctx->skip_frame = AVDISCARD_NONE;
		break;
	case 1:
		pc->avctx->skip_frame = AVDISCARD_BIDIR;
		break;
	case 2:
	case 3:
		pc->avctx->skip_frame = AVDISCARD_NONKEY;
		break;
	case 4:
		pc->avctx->skip_frame = AVDISCARD_NONREF | AVDISCARD_BIDIR;
		break;
	case 5:
	case 6:
	case 7:
		pc->avctx->skip_frame = AVDISCARD_ALL;
	}
}

static void InitSpecialPorts(void)
{
    AVCodec* codec = 0;
    PortContext* pc = 0;
    // Init bmp image converter
    pc = GetPortContext(XPR_MCDEC_SPEC_BMPENC_PORT);
	memset(pc->out_stblk_handlers, 0, sizeof(pc->out_stblk_handlers));
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
        pc->avpkt_buf_size = 8000000;
        pc->avpkt_buf = (uint8_t*)XPR_Alloc(pc->avpkt_buf_size);
        pc->params.image_quality = 100;
    }
    // Init jpeg image converter
	pc = GetPortContext(XPR_MCDEC_SPEC_JPGENC_PORT);
	memset(pc->out_stblk_handlers, 0, sizeof(pc->out_stblk_handlers));
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
        pc->avpkt_buf_size = 1000000;
        pc->avpkt_buf = (uint8_t*)XPR_Alloc(pc->avpkt_buf_size);
        pc->params.image_quality = 100;
    }
    // Init png image converter
	pc = GetPortContext(XPR_MCDEC_SPEC_PNGENC_PORT);
	memset(pc->out_stblk_handlers, 0, sizeof(pc->out_stblk_handlers));
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
        pc->avpkt_buf_size = 1000000;
        pc->avpkt_buf = (uint8_t*)XPR_Alloc(pc->avpkt_buf_size);
        pc->params.image_quality = 80;
    }
}

static int F2A(uint32_t fourcc)
{
	switch (fourcc) {
	case AV_FOURCC_H264:
		return AV_CODEC_ID_H264;
	case AV_FOURCC_H265:
		return AV_CODEC_ID_HEVC;
	case AV_FOURCC_JPEG:
		return AV_CODEC_ID_MJPEG;
	}
	return AV_CODEC_ID_NONE;
}

static int ResetPort(int port, uint32_t fourcc)
{
	int major = XPR_MCDEC_PORT_MAJOR(port);
	int minor = XPR_MCDEC_PORT_MINOR(port);
	AVCodec* codec = 0;
	PortContext* pc = 0;
	//
	if (major == XPR_MCDEC_PORT_MAJOR_GEN) {
		pc = &xpr_mcdec_ports[major][minor];
		if (pc->cur_fourcc != 0 && pc->cur_fourcc != fourcc) {
			avcodec_close(pc->avctx);
			av_frame_free(&pc->avfrm);
			if (pc->out_frame_buffer) {
				free(pc->out_frame_buffer);
				pc->out_frame_buffer = 0;
				memset(&pc->out_frame, 0, sizeof(pc->out_frame));
			}
			pc->flags &= XPR_MCDEC_FLAG_BLKED;
			pc->avctx = 0;
			pc->cur_fourcc = 0;
		}
		codec = avcodec_find_decoder(F2A(fourcc));
		if (!codec) {
			fprintf(stderr, "XPR_MCDEC: avcodec_find_decoder(%x) failure\n", F2A(fourcc));
			return XPR_ERR_ERROR;
		}
		//fprintf(stderr, "XPR_MCDEC: avcodec_find_decoder(%x) = %p\n", F2A(fourcc), codec);
		pc->flags &= XPR_MCDEC_FLAG_BLKED;
		pc->flags |= XPR_MCDEC_FLAG_SYNRQ;
		pc->avctx = avcodec_alloc_context3(codec);
		pc->avfrm = av_frame_alloc();
		if (!pc->out_frame_buffer)
			pc->out_frame_buffer = malloc(10*1024*1024); // 10M
		pc->out_frame.datas[0] = pc->out_frame_buffer;
		pc->out_frame.datas[1] = pc->out_frame.datas[0] + 1920 * 1088;
		pc->out_frame.datas[2] = pc->out_frame.datas[1] + 1920 * 1080 / 4;
		//
		pc->avctx->delay = 0;
		SetupSkipFrame(pc);//pc->avctx->skip_frame = AVDISCARD_NONKEY;
		pc->avctx->thread_count = 2;
		pc->avctx->thread_type = FF_THREAD_FRAME | FF_THREAD_SLICE;
		pc->avctx->refcounted_frames = 0;
		avcodec_open2(pc->avctx, codec, NULL);
		pc->cur_fourcc = fourcc;
	}
	return XPR_ERR_OK;
}

int XPR_MCDEC_Config(XPR_MCDEC_CFG cfg, const void* data, int size)
{
	switch (cfg) {
	case XPR_MCDEC_CFG_C4WGA:
		xpr_mcdec_cfg.c4wga = size ? *(int*)data : (int)data;
		break;
	case XPR_MCDEC_CFG_LOG_LEVEL:
		xpr_mcdec_cfg.log_level = size ? *(int*)data : (int)data;
		break;
    default:
        break;
	}
	return 0;
}

int XPR_MCDEC_Init(void)
{
    int i = XPR_MCDEC_CHANNEL_MIN;
    long errorCode = 0;
    AVCodec* codec = 0;
    PortContext* pc = 0;
    //
#if defined(_M_X64)
	_set_FMA3_enable(0);
#endif
#if defined(WIN32) || defined(WIN64)
	if (xpr_mcdec_cfg.c4wga) {
		AllocConsole();
		freopen("conin$", "r+t", stdin);
		freopen("conout$", "w+t", stdout);
		freopen("conout$", "w+t", stderr);
	}
#endif
    //
	memset(&xpr_mcdec_avfrm_handlers, 0, sizeof(xpr_mcdec_avfrm_handlers));
	memset(&xpr_mcdec_stblk_handlers, 0, sizeof(xpr_mcdec_stblk_handlers));
	memset(&xpr_mcdec_ports, 0, sizeof(xpr_mcdec_ports));
	av_log_set_level(xpr_mcdec_cfg.log_level);
    avcodec_register_all();
    //
    InitSpecialPorts();
    //
    return 0;
}

int XPR_MCDEC_Fini(void)
{
    int i = XPR_MCDEC_CHANNEL_MIN;
    for (; i <= XPR_MCDEC_CHANNEL_MAX; i++) {
		ResetPort(XPR_MCDEC_PORT(1, i), 0);
    }
    return 0;
}

int XPR_MCDEC_Flush(int port)
{
	if (!IsPortValid(port))
		return XPR_ERR_ERROR;
	return FlushPort(port);
}

int XPR_MCDEC_Reset(int port)
{
	PortContext* pc = 0;
	if (!IsPortValid(port))
		return XPR_ERR_ERROR;
	pc = GetPortContext(port);
	if (pc->flags & XPR_MCDEC_FLAG_PNDNG)
		pc->flags |= XPR_MCDEC_FLAG_CLOSE;
	else
		ResetPort(port, 0);
	return XPR_ERR_SUCCESS;
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
    //AVPicture avpic;
	uint8_t *pointers[4];
	int linesizes[4];
    XPR_StreamBlock stblk;
    struct SwsContext* swsctx = 0;
    PortContext* pc = GetPortContext(port);
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
        avpkt.data = pc->avpkt_buf;
        avpkt.size = pc->avpkt_buf_size;
        //
        pc->avctx->qmin = MIN(100, MAX(1, 100 - pc->params.image_quality));
        //
        if (port == XPR_MCDEC_SPEC_BMPENC_PORT && frm->format != pc->avctx->pix_fmt)
            force_scaled =1;

        if ((pc->params.image_width == 0 || pc->params.image_height == 0) && !force_scaled) {
            pc->avctx->width = frm->width;
            pc->avctx->height = frm->height;
        }
        else {
            pc->avctx->width = pc->params.image_width ? pc->params.image_width : frm->width;
            pc->avctx->height = pc->params.image_height ? pc->params.image_height : frm->height;
            dst_pix_fmt = port == XPR_MCDEC_SPEC_BMPENC_PORT ? AV_PIX_FMT_BGR24 : avfrm.format;
            swsctx = sws_getContext(avfrm.width, avfrm.height, (enum AVPixelFormat)avfrm.format, 
                                    pc->avctx->width, pc->avctx->height, (enum AVPixelFormat)dst_pix_fmt,
                                    SWS_BICUBIC, NULL, NULL, NULL);
            if (swsctx) {
				if (av_image_alloc(pointers, linesizes,
							   pc->avctx->width, pc->avctx->height,
							   (enum AVPixelFormat)dst_pix_fmt, 4) > 0) {
                    if (sws_scale(swsctx, (const uint8_t * const*)avfrm.data, avfrm.linesize, 0, avfrm.height, pointers, linesizes) > 0)
                        scaled = 1;
                    else
                        av_freep(&pointers[0]);
                    sws_freeContext(swsctx);
                    //
                    avfrm.data[0] = pointers[0];
                    avfrm.data[1] = pointers[1];
                    avfrm.data[2] = pointers[2];
                    avfrm.linesize[0] = linesizes[0];
                    avfrm.linesize[1] = linesizes[1];
                    avfrm.linesize[2] = linesizes[2];
                    avfrm.extended_data = (uint8_t**)pointers;
                    avfrm.width = pc->avctx->width;
                    avfrm.height = pc->avctx->height;
                    avfrm.format = dst_pix_fmt;
                }
            }
        }
        if (avcodec_open2(pc->avctx, pc->avcodec, NULL) < 0) {
            if (scaled)
				av_freep(&pointers[0]);;
            return XPR_ERR_ERROR;
        }
        //
        avcodec_encode_video2(pc->avctx, &avpkt, &avfrm, &got);
        avcodec_close(pc->avctx);
        if (scaled)
			av_freep(&pointers[0]);
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

#if defined(WIN32) || defined(WIN64)
long WINAPI FilterFunc(DWORD dwExceptionCode)
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#define TRACE_LINE()
//#define TRACE_LINE()	fprintf(stderr, "!!! DEBUG: %s:%d(%s)\n", __FILE__, __FUNCTION__, __LINE__)

int XPR_MCDEC_PushData(int port, const uint8_t* data, int length)
{
    int got = 0;
	PortContext* pc = 0;
    AVPacket pkt;
    XPR_AVFrame out_avf;;
    //
    if (!IsPortValid(port))
        return XPR_ERR_ERROR;
	if (!data || length < 4)
		return XPR_ERR_ERROR;
	//
	pc = GetPortContext(port);
	if (!IsPortReady(port, pc->cur_fourcc))
		return XPR_ERR_ERROR;
	if (!pc->avctx || !pc->avfrm)
		return XPR_ERR_ERROR;
	//
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)data;
    pkt.size = length;
    pkt.dts = AV_NOPTS_VALUE;
    pkt.pts = AV_NOPTS_VALUE;
#if defined(WIN32) || defined(WIN64)
	__try {
#endif    
		avcodec_decode_video2(pc->avctx, pc->avfrm, &got, &pkt);
#if defined(WIN32) || defined(WIN64)
	}
	__except (FilterFunc(GetExceptionCode())) {
		//
		//OutputDebugStringA("RAISED\n");
		return XPR_ERR_ERROR;
	}
#endif
	if (got) {
		out_avf.datas[0] = pc->avfrm->data[0];
		out_avf.datas[1] = pc->avfrm->data[2];
		out_avf.datas[2] = pc->avfrm->data[1];
		out_avf.pitches[0] = pc->avfrm->linesize[0];
		out_avf.pitches[1] = pc->avfrm->linesize[1];
		out_avf.pitches[2] = pc->avfrm->linesize[2];
		out_avf.format = XPR_AV_PIX_FMT_YUV420P;
		out_avf.width = pc->avfrm->width;
		out_avf.height = pc->avfrm->height;
		XPR_MCDEC_DeliverAVFrame(port, &out_avf);
	}

    return XPR_ERR_SUCCESS;
}

int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* blk)
{
    int i = 0;
    int got = 0;
	PortContext* pc = 0;
    AVPacket pkt = {0};
    //XPR_AVFrame out_avf;
    //

	if (!blk)
		return XPR_ERR_ERROR;
    if (!IsPortValid(port))
        return XPR_ERR_ERROR;
	if (IsPortBlocked(port) || IsPortClosed(port))
		return XPR_ERR_ERROR;
	if (!IsPortReady(port, blk->codec)) {
		if (ResetPort(port, blk->codec) != XPR_ERR_OK)
			return XPR_ERR_ERROR;
	}
	pc = GetPortContext(port);
    if (!pc->avctx || !pc->avfrm)
		return XPR_ERR_ERROR;
	//
	if (pc->flags & XPR_MCDEC_FLAG_SYNRQ) {
		if (!(XPR_StreamBlockFlags(blk) & XPR_STREAMBLOCK_FLAG_TYPE_I))
			return XPR_ERR_OK;
		pc->flags &= ~XPR_MCDEC_FLAG_SYNRQ;
	}
	pc->flags |= XPR_MCDEC_FLAG_PNDNG;
    av_init_packet(&pkt);
    pkt.data = (uint8_t*)XPR_StreamBlockData(blk);
    pkt.size = (int)XPR_StreamBlockLength(blk);
    pkt.dts = AV_NOPTS_VALUE;
    pkt.pts = AV_NOPTS_VALUE;
	pkt.flags |= (XPR_StreamBlockFlags(blk) & XPR_STREAMBLOCK_FLAG_TYPE_I) ? AV_PKT_FLAG_KEY : 0;
	//printf("%p, %d, %02hhx, %02hhx, %02hhx, %02hhx, %02hhx, %02hhx\n",
	//       pkt.data, pkt.size, pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4], pkt.data[5]);
#if defined(WIN32) || defined(WIN64)
	__try {
#endif
		i = avcodec_decode_video2(pc->avctx, pc->avfrm, &got, &pkt);
#if defined(WIN32) || defined(WIN64)
	}
	__except (FilterFunc(GetExceptionCode())) {
		//
		//OutputDebugStringA("RAISED\n");
		return XPR_ERR_ERROR;
	}
#endif
    if (got) {
		if (pc->params.deinterlace) {
			//printf("field order: %d\n", pc->avctx->field_order);
			memcpy(pc->out_frame.datas[0], pc->avfrm->data[0], pc->avfrm->height * pc->avfrm->linesize[0]);
			memcpy(pc->out_frame.datas[1], pc->avfrm->data[1], pc->avfrm->height * pc->avfrm->linesize[1]);
			memcpy(pc->out_frame.datas[2], pc->avfrm->data[2], pc->avfrm->height * pc->avfrm->linesize[2]);
			//pc->out_frame.datas[0] = pc->avfrm->data[0];
			//pc->out_frame.datas[1] = pc->avfrm->data[1];
			//pc->out_frame.datas[2] = pc->avfrm->data[2];
			pc->out_frame.pitches[0] = pc->avfrm->linesize[0];
			pc->out_frame.pitches[1] = pc->avfrm->linesize[1];
			pc->out_frame.pitches[2] = pc->avfrm->linesize[2];
			pc->out_frame.format = XPR_AV_PIX_FMT_YUV420P;
			pc->out_frame.width = pc->avfrm->width;
			pc->out_frame.height = pc->avfrm->height;
			XPR_MCDEC_DeliverAVFrame(port, &pc->out_frame);
		}
		else {
			memcpy(pc->out_frame.datas[0], pc->avfrm->data[0], pc->avfrm->height * pc->avfrm->linesize[0]);
			memcpy(pc->out_frame.datas[1], pc->avfrm->data[1], pc->avfrm->height / 2 * pc->avfrm->linesize[1]);
			memcpy(pc->out_frame.datas[2], pc->avfrm->data[2], pc->avfrm->height / 2 * pc->avfrm->linesize[2]);
			//pc->out_frame.datas[0] = pc->avfrm->data[0];
			//pc->out_frame.datas[1] = pc->avfrm->data[2];
			//pc->out_frame.datas[2] = pc->avfrm->data[1];
			pc->out_frame.pitches[0] = pc->avfrm->linesize[0];
			pc->out_frame.pitches[1] = pc->avfrm->linesize[1];
			pc->out_frame.pitches[2] = pc->avfrm->linesize[2];
			pc->out_frame.format = XPR_AV_PIX_FMT_YUV420P;
			pc->out_frame.width = pc->avfrm->width;
			pc->out_frame.height = pc->avfrm->height;
			XPR_MCDEC_DeliverAVFrame(port, &pc->out_frame);
		}
    }
	pc->flags &= ~XPR_MCDEC_FLAG_PNDNG;
	if (pc->flags & XPR_MCDEC_FLAG_CLOSE)
		ResetPort(port, 0);
	return XPR_ERR_OK;
}

void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avfrm)
{
    int i = 0;
	PortContext* pc = NULL;

	if (port != 0)
		pc = GetPortContext(port);

    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
		if (port == 0) {
			if (xpr_mcdec_avfrm_handlers[i].handler) {
				((XPR_MCDEC_AVF_FXN)xpr_mcdec_avfrm_handlers[i].handler)(
					xpr_mcdec_avfrm_handlers[i].opaque, port, avfrm);
			}
		}
		else if (pc) {
			if (pc->out_frame_handlers[i].handler) {
				((XPR_MCDEC_AVF_FXN)pc->out_frame_handlers[i].handler)(
					pc->out_frame_handlers[i].opaque, port, avfrm);
			}
		}
    }
}

void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* blk)
{
    int i = 0;
	PortContext* pc = NULL;

	if (port != 0)
		pc = GetPortContext(port);

    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (xpr_mcdec_stblk_handlers[i].handler) {
            ((XPR_MCDEC_STB_FXN)xpr_mcdec_stblk_handlers[i].handler)(
                    xpr_mcdec_stblk_handlers[i].opaque, port, blk);
        }
		else if (pc) {
			if (pc->out_stblk_handlers[i].handler) {
				((XPR_MCDEC_STB_FXN)pc->out_stblk_handlers[i].handler)(
					pc->out_stblk_handlers[i].opaque, port, blk);
			}
		}
    }
}

int XPR_MCDEC_AddAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque)
{
	int i = 0;
	PortContext* pc = 0;
	if (port == 0) {
		xpr_mcdec_avfrm_handlers[0].handler = handler;
		xpr_mcdec_avfrm_handlers[0].opaque = opaque;
		return XPR_ERR_SUCCESS;
	}
	pc = GetPortContext(port);
	if (!pc)
		return XPR_ERR_ERROR;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
        if (pc->out_frame_handlers[i].handler == 0) {
			pc->out_frame_handlers[i].handler = handler;
			pc->out_frame_handlers[i].opaque = opaque;
            return XPR_ERR_SUCCESS;
        }
    }
    return XPR_ERR_ERROR;
}

int XPR_MCDEC_DelAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque)
{
	int i = 0;
	PortContext* pc = 0;
	if (port == 0) {
		if (xpr_mcdec_avfrm_handlers[0].handler == handler &&
			xpr_mcdec_avfrm_handlers[0].opaque == opaque) {
			xpr_mcdec_avfrm_handlers[0].handler = NULL;
			xpr_mcdec_avfrm_handlers[0].opaque = NULL;
		}
		return XPR_ERR_SUCCESS;
	}
	pc = GetPortContext(port);
	if (!pc)
		return XPR_ERR_ERROR;
	for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
		if (pc->out_frame_handlers[i].handler == handler &&
			pc->out_frame_handlers[i].opaque == opaque) {
			pc->out_frame_handlers[i].handler = NULL;
			pc->out_frame_handlers[i].opaque = NULL;
			return XPR_ERR_SUCCESS;
		}
	}
	return XPR_ERR_ERROR;
}

int XPR_MCDEC_AddStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque)
{
    int i = 0;
	PortContext* pc = 0;
	if (port == 0) {
		xpr_mcdec_stblk_handlers[0].handler = handler;
		xpr_mcdec_stblk_handlers[0].opaque = opaque;
		return XPR_ERR_SUCCESS;
	}
	pc = GetPortContext(port);
	if (!pc)
		return XPR_ERR_GEN_SYS_NOTREADY;
    for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
		if (pc->out_stblk_handlers[i].handler == 0) {
			pc->out_stblk_handlers[i].handler = handler;
			pc->out_stblk_handlers[i].opaque = opaque;
            return XPR_ERR_SUCCESS;
        }
    }
    return XPR_ERR_GEN_UNEXIST;
}

int XPR_MCDEC_DelStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque)
{
	int i = 0;
	PortContext* pc = 0;
	if (port == 0) {
		if (xpr_mcdec_stblk_handlers[0].handler == handler &&
			xpr_mcdec_stblk_handlers[0].opaque == opaque) {
			xpr_mcdec_stblk_handlers[0].handler = NULL;
			xpr_mcdec_stblk_handlers[0].opaque = NULL;
		}
		return XPR_ERR_SUCCESS;
	}
	pc = GetPortContext(port);
	if (!pc)
		return XPR_ERR_GEN_SYS_NOTREADY;
	for (; i < XPR_MCDEC_MAX_HANDLERS; i++) {
		if (pc->out_stblk_handlers[i].handler == handler &&
			pc->out_stblk_handlers[i].opaque == opaque) {
			pc->out_stblk_handlers[i].handler = NULL;
			pc->out_stblk_handlers[i].opaque = NULL;
			return XPR_ERR_SUCCESS;
		}
	}
	return XPR_ERR_GEN_UNEXIST;
}

int XPR_MCDEC_SetParam(int port, XPR_MCDEC_PARAM param, const void* data, int size)
{
    PortContext* pc = 0;
    if (!IsPortValid(port))
        return XPR_ERR_ERROR;
    pc = GetPortContext(port);
    switch (param) {
	case XPR_MCDEC_PARAM_BLOCK:
		pc->params.block = size ? *(uint32_t*)data : (uint32_t)data;
		break;
	case XPR_MCDEC_PARAM_DEINTERLANCE:
		pc->params.deinterlace = size ? *(uint32_t*)data : (uint32_t)data;
		break;
	case XPR_MCDEC_PARAM_FORCE_FOURCC:
		pc->params.force_fourcc = size ? *(uint32_t*)data : (uint32_t)data;
		if (pc->cur_fourcc != pc->params.force_fourcc)
			ResetPort(port, pc->params.force_fourcc);
		break;
    case XPR_MCDEC_PARAM_IMAGE_WIDTH:
        pc->params.image_width = size ? *(int*)data : (int)data;
        break;
    case XPR_MCDEC_PARAM_IMAGE_HEIGHT:
        pc->params.image_height = size ? *(int*)data : (int)data;
        break;
    case XPR_MCDEC_PARAM_IMAGE_QUALITY:
        pc->params.image_quality = size ? *(int*)data : (int)data;
        break;
	case XPR_MCDEC_PARAM_SKIP_FRAME:
		pc->params.skip_frame = size ? *(uint32_t*)data : (uint32_t)data;
		if (IsPortReady(port, pc->cur_fourcc))
			SetupSkipFrame(pc);
		break;
    default:
        break;
    }
    return XPR_ERR_SUCCESS;
}

int XPR_MCDEC_GetParam(int port, XPR_MCDEC_PARAM param, void* data, int* size)
{
    return XPR_ERR_SUCCESS;
}

#endif // HAVE_XPR_MCDEC_DRIVER_LIBAV