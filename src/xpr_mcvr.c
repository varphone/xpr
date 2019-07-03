#if !defined(XPR_MCVR_DRIVER_D3D)
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mcvr.h>
#include <xpr/xpr_mem.h>

// Dummy interfaces
//============================================================================
XPR_API int XPR_MCVR_GetLastError(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API char* XPR_MCVR_GetErrorString(void)
{
    return "Not Supported";
}

XPR_API wchar_t* XPR_MCVR_GetErrorStringW(void)
{
    return L"Not Supported";
}

XPR_API int XPR_MCVR_Init(void* hwnd, XPR_MCVR_VideoRendererType type)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_Fini(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API XPR_MCVR_VideoRendererType XPR_MCVR_GetVideoRendererType(void)
{
    return XPR_MCVR_VIDEO_RENDERER_NULL;
}

XPR_API int XPR_MCVR_BindWindow(int main_port, void* hwnd, int rows, int cols)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_InputData(int port, XPR_AVFrame* av_frame)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_ZoomIn(int src_port, int dest_port)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_GetZoomPort(int src_port)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_ResetPort(int port)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_Snapshot(int port, const char* path)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_GetVideoSize(int port, int* width, int* height)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetAspectRatio(float ratio)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API float XPR_MCVR_GetAspectRatio()
{
    return 0.0;
}

XPR_API int XPR_MCVR_SetScale(int port, float factor)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API float XPR_MCVR_GetScale(int port)
{
    return 0.0;
}

XPR_API int XPR_MCVR_SetRate(int port, float rate)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetRenderLevel(XPR_MCVR_RenderLevel level)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API XPR_MCVR_RenderLevel XPR_MCVR_GetRenderLevel(void)
{
    return XPR_MCVR_RENDER_LEVEL_0;
}

XPR_API int XPR_MCVR_SetString(int port, XPR_MCVR_StringType type,
                               char* strings)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetStrings(int port, XPR_MCVR_StringType type,
                                char** strings, int count)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetStringState(int port, XPR_MCVR_StringType type,
                                    XPR_MCVR_StringState state)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API XPR_MCVR_StringState XPR_MCVR_GetStringState(int port,
                                                     XPR_MCVR_StringType type)
{
    return XPR_MCVR_STRING_STATE_HIDE;
}

XPR_API int XPR_MCVR_GetCurrentPort(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetChannelState(int port, XPR_MCVR_ChannelState state)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API XPR_MCVR_ChannelState XPR_MCVR_GetChannelState(int port)
{
    return XPR_MCVR_CHANNEL_NO_CAMERA;
}

XPR_API int XPR_MCVR_SetEffect(int port, XPR_MCVR_EffectType effect, int value)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_SetEffectF(int port, XPR_MCVR_EffectType effect,
                                float value)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_GetEffect(int port, XPR_MCVR_EffectType effect)
{
    return 0;
}

XPR_API float XPR_MCVR_GetEffectF(int port, XPR_MCVR_EffectType effect)
{
    return 0.0;
}

XPR_API int XPR_MCVR_AddEventCallback(XPR_MCVR_EventCallback callback,
                                      void* user)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_DelEventCallback(XPR_MCVR_EventCallback callback,
                                      void* user)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_AttachEvent(XPR_MCVR_EventType ev)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_AttachAllEvents(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_DetachEvent(XPR_MCVR_EventType ev)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_MCVR_DetachAllEvents(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}
#endif // !XPR_MCVR_DRIVER_D3D
