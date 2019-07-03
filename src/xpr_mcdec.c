#if !defined(HAVE_XPR_MCDEC_DRIVER_LIBAV)

#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mcdec.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_utils.h>

XPR_API int XPR_MCDEC_Config(XPR_MCDEC_CFG cfg, const void* data, int size)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_Init(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_Fini(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_Flush(int port)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_Reset(int port)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* frm)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_PushData(int port, const uint8_t* data, int length)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* blk)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avfrm)
{
}

XPR_API void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* blk)
{
}

XPR_API int XPR_MCDEC_AddAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler,
                                        void* opaque)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_AddStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler,
                                            void* opaque)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_SetParam(int port, XPR_MCDEC_PARAM param,
                               const void* data, int size)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCDEC_GetParam(int port, XPR_MCDEC_PARAM param, void* data,
                               int* size)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

#endif // !defined(HAVE_XPR_MCDEC_DRIVER_LIBAV)
