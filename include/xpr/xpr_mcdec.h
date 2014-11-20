#ifndef XPR_MCDEC_H
#define XPR_MCDEC_H

#include <stdint.h>
#include "xpr_avframe.h"
#include "xpr_streamblock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_MCDEC_MAX_HANDLERS          8

#define XPR_MCDEC_PORT(major, minor)    (((major)<<16)|(minor))
#define XPR_MCDEC_PORT_MAJOR(port)      (((port)>>16) & 0xffff)
#define XPR_MCDEC_PORT_MINOR(port)      ((port) & 0xffff)

#define XPR_MCDEC_SPEC_BMPENC_PORT      (0x00000001)
#define XPR_MCDEC_SPEC_JPGENC_PORT      (0x00000002)
#define XPR_MCDEC_SPEC_PNGENC_PORT      (0x00000003)

typedef int (*XPR_MCDEC_AVFrameHandler)(void* opaque, int port, const XPR_AVFrame* frm);
typedef int (*XPR_MCDEC_StreamBlockHandler)(void* opaque, int port, const XPR_StreamBlock* blk);

typedef enum XPR_MCDEC_Param {
    XPR_MCDEC_PARAM_IMAGE_WIDTH,
    XPR_MCDEC_PARAM_IMAGE_HEIGHT,
    XPR_MCDEC_PARAM_IMAGE_QUALITY,
};

int XPR_MCDEC_Config(int option, const void* data, int length);
int XPR_MCDEC_Init(void);
int XPR_MCDEC_Fini(void);

int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* frm);
int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* blk);
int XPR_MCDEC_PushData(int port, const uint8_t* data, int length);

int XPR_MCDEC_Reset(int port);

void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avfrm);
void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* blk);

int XPR_MCDEC_AddAVFrameHandler(XPR_MCDEC_AVFrameHandler handler, void* opaque);
int XPR_MCDEC_AddStreamBlockHandler(XPR_MCDEC_StreamBlockHandler handler, void* opaque);

int XPR_MCDEC_SetParam(int port, int param, const void* data, int size);
int XPR_MCDEC_GetParam(int port, int param, void* data, int* size);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_H
