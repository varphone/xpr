#if !defined(XPR_MCDEC_DRIVER_LIBAV)

#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_mcdec.h>
#include <xpr/xpr_utils.h>

int XPR_MCDEC_Config(XPR_MCDEC_CFG cfg, const void* data, int size)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_Init(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_Fini(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_Flush(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_Reset(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* frm)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_PushData(int port, const uint8_t* data, int length)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* blk)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avfrm)
{
}

void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* blk)
{
}

int XPR_MCDEC_AddAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_AddStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_SetParam(int port, XPR_MCDEC_PARAM param, const void* data, int size)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_MCDEC_GetParam(int port, XPR_MCDEC_PARAM param, void* data, int* size)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

#endif // !XPR_MCDEC_DRIVER_LIBAV