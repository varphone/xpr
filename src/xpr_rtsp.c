#if !defined(XPR_RTSP_DRIVER_LIVE)
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>

int XPR_RTSP_Init(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_Fini(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_IsPortValid(int port)
{
	return XPR_FALSE;
}

int XPR_RTSP_Open(int port, const char* url)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_Close(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_Start(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_Stop(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

#if defined(XPR_RTSP_XD_STREAM_API)
int XPR_RTSP_AddDataCallbackXD(int port, XD_StreamDataCallback cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_DelDataCallbackXD(int port, XD_StreamDataCallback cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_AddEventCallbackXD(int port, XD_StreamEventCallback cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_DelEventCallbackXD(int port, XD_StreamEventCallback cb, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}
#endif // defined(XPR_RTSP_XD_STREAM_API)

int XPR_RTSP_SetAuth(int port, const char* username, const char* password, int pwdIsMD5)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_SetOutputFormat(int port, int fourcc)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_PostData(int port, XPR_StreamBlock* stb)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_PostEvent(int port, const XPR_RTSP_EVD* evd)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_SetTrSpec(int port, XPR_RTSP_TRSPEC trspec)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_GetParam(int port, XPR_RTSP_PARAM param, void* buffer, int* size)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XPR_RTSP_SetParam(int port, XPR_RTSP_PARAM param, const void* data, int length)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

#if defined(XPR_RTSP_XD_STREAM_API)
void libXD_Stream_Config(int request, const char* optVal, size_t optLen)
{
}

int libXD_Stream_Init(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

void libXD_Stream_Fini(void)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

const char* libXD_Stream_Version(void)
{
    return "Not Supported";
}

int libXD_Stream_VersionNumber(void)
{
    return 0xffffffff;
}

int XD_StreamOpen(const char *url)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamClose(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamStart(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamStop(int port)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamAddDataCallback(int port, XD_StreamDataCallback callback, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamDelDataCallback(int port, XD_StreamDataCallback callback, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamAddEventCallback(int port, XD_StreamEventCallback callback, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamDelEventCallback(int port, XD_StreamEventCallback callback, void* opaque)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamSetAuth(int port, const char* username, const char* password, int pwdIsMD5)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamSetOFPM(int port, int yes)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamSetOutputFormat(int port, int fourcc)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamSetTimeout(int port, int msecs)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

int XD_StreamSetTransferMode(int port, int mode)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}
#endif // defined(XPR_RTSP_XD_STREAM_API)
#endif // !defined(XPR_RTSP_DRIVER_LIVE)