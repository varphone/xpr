#include <math.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_mcvr.h>

#ifdef HAVE_XPR_MCVR_DRIVER_D3D
#pragma comment(lib, d3d9.lib)
#pragma comment(lib, d3dx9.lib)

#include "drivers/mcvr/d3d9/d3drenderer.hpp"
#include "drivers/mcvr/d3d9/safefree.hpp"

XD::D3DRenderer * _mcvr_renderer = nullptr;

XPR_MCVR_VideoRendererType _renderer_type = XPR_MCVR_VIDEO_RENDERER_DEFAULT;

int _last_mcvr_error = XPR_MCVR_ERR_NONE;

int XPR_MCVR_GetLastError(void)
{
    return _last_mcvr_error;
}

char *XPR_MCVR_GetErrorString(void)
{
    char *error_string = reinterpret_cast<char*>(XPR_Alloc(100));
    if(!error_string) {
        return nullptr;
    }
    switch(_last_mcvr_error) {
    case XPR_MCVR_ERR_NONE:
        sprintf_s(error_string, 100, "MCVR:No error.\n");
        break;
    case XPR_MCVR_ERR_UNKNOWN:
        sprintf_s(error_string, 100, "MCVR:Unknown error.\n");
        break;
    case XPR_MCVR_ERR_RENDERER_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create renderer core.\n");
        break;
    case XPR_MCVR_ERR_NULL_RENDERER:
        sprintf_s(error_string, 100, "MCVR:Error on null renderer core.\n");
        break;
    case XPR_MCVR_ERR_DEVICE_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create device.\n");
        break;
    case XPR_MCVR_ERR_VERTEX_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create vertex buffer.\n");
        break;
    case XPR_MCVR_ERR_EFFECT_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create effect.\n");
        break;
    case XPR_MCVR_ERR_THREAD_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create thread.\n");
        break;
    case XPR_MCVR_ERR_TEXTURE_CREATE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to create texture.\n");
        break;
    case XPR_MCVR_ERR_IMAGE_ENHANCE_FAILED:
        sprintf_s(error_string, 100, "MCVR:Error on failing to enhance image.\n");
        break;
    case XPR_MCVR_ERR_NULL_DEVICE:
        sprintf_s(error_string, 100, "MCVR:Error on null device.\n");
        break;
    case XPR_MCVR_ERR_NULL_CHANNEL:
        sprintf_s(error_string, 100, "MCVR:Error on null channel.\n");
        break;
    case XPR_MCVR_ERR_NULL_TEXTURE:
        sprintf_s(error_string, 100, "MCVR:Error on null texture.\n");
        break;
    case XPR_MCVR_ERR_INVALID_PORT:
        sprintf_s(error_string, 100, "MCVR:Error on invalid port.\n");
        break;
    case XPR_MCVR_ERR_INVALID_LAYOUT:
        sprintf_s(error_string, 100, "MCVR:Error on invalid layout.\n");
        break;
    case XPR_MCVR_ERR_INVALID_FACTOR:
        sprintf_s(error_string, 100, "MCVR:Error on invalid image factor.\n");
        break;
    case XPR_MCVR_ERR_INVALID_PIXEL_FORMAT:
        sprintf_s(error_string, 100, "MCVR:Error on invalid pixel format.\n");
        break;
    case XPR_MCVR_ERR_INVALID_INPUT_DATA:
        sprintf_s(error_string, 100, "MCVR:Error on invalid input data.\n");
        break;
    default:
        sprintf_s(error_string, 100, "MCVR:No error.\n");
        break;
    }

    return error_string;
}

static wchar_t *charToWChar(wchar_t *dest, const char *src)
{
    if(dest && src) {
        DWORD size = MultiByteToWideChar(CP_ACP, 0, src, -1, nullptr, 0);
        MultiByteToWideChar(CP_ACP, 0, src, -1, dest, size);
        return dest;
    }

    return nullptr;
}

wchar_t *XPR_MCVR_GetErrorStringW(void)
{
    wchar_t *error_string = reinterpret_cast<wchar_t*>(XPR_Alloc(100*sizeof(wchar_t)));
    if(!error_string) {
        return nullptr;
    }

    char *str = XPR_MCVR_GetErrorString();
    if(charToWChar(error_string, str)) {
        XPR_Free(str);
        return error_string;
    }
    XPR_Free(str);
    XPR_Free(error_string);
    return nullptr;
}

int XPR_MCVR_Init(void *hwnd, XPR_MCVR_VideoRendererType type)
{
#if defined(_M_X64)
	_set_FMA3_enable(0);
#endif
	//
    if(_mcvr_renderer) {
        return 1;
    }

    _renderer_type = type;
    switch(type)
    {
    case XPR_MCVR_VIDEO_RENDERER_DEFAULT:
        _renderer_type = XPR_MCVR_VIDEO_RENDERER_D3D;
    case XPR_MCVR_VIDEO_RENDERER_D3D:
        {
            _mcvr_renderer = new XD::D3DRenderer(hwnd);
            if(_mcvr_renderer) {
                _last_mcvr_error = XPR_MCVR_ERR_NONE;
                return 1;
            }
        }
        break;
    case XPR_MCVR_VIDEO_RENDERER_NULL:
        return 1;
    case XPR_MCVR_VIDEO_RENDERER_OPENGL:
        break;
    default:
        break;
    }

    _last_mcvr_error = XPR_MCVR_ERR_RENDERER_CREATE_FAILED;
    return 0;
}

void XPR_MCVR_Fini(void)
{
    SafeDelete(&_mcvr_renderer);
    _last_mcvr_error = XPR_MCVR_ERR_NONE;
    _renderer_type = XPR_MCVR_VIDEO_RENDERER_DEFAULT;
}

XPR_MCVR_VideoRendererType XPR_MCVR_GetVideoRendererType(void)
{
    return _renderer_type;
}

#define CHECK_RENDERER_RETVAL(val) \
    if(!_mcvr_renderer) { \
        _last_mcvr_error = XPR_MCVR_ERR_NULL_RENDERER; \
        return val; \
    }
#define CHECK_RENDERER_NORET() \
    if(!_mcvr_renderer) { \
        _last_mcvr_error = XPR_MCVR_ERR_NULL_RENDERER; \
        return; \
    }

int XPR_MCVR_BindWindow(int main_port, void *hwnd, int rows, int cols)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->bindPort(main_port, hwnd, rows, cols) ? 1 : 0;
}

int XPR_MCVR_InputData(int port, XPR_AVFrame *av_frame)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->inputData(port, av_frame) ? 1 : 0;
}

int XPR_MCVR_ZoomIn(int src_port, int dest_port)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->zoomIn(src_port, dest_port) ? 1 : 0;
}

int XPR_MCVR_GetZoomPort(int src_port)
{
    CHECK_RENDERER_RETVAL(-1);
    return _mcvr_renderer->getZoomPort(src_port);
}

int XPR_MCVR_ResetPort(int port)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->resetPort(port) ? 1 : 0;
}

int XPR_MCVR_Snapshot(int port, const char *path)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->snapShot(port, path) ? 1 : 0;
}

int XPR_MCVR_GetVideoSize(int port, int *width, int *height)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->getVideoSize(port, width, height) ? 1 : 0;
}

//void XPR_MCVR_SetAspectRatio(float ratio)
void XPR_MCVR_SetAspectRatio(float ratio)
{
    CHECK_RENDERER_NORET();
    _mcvr_renderer->setAspectRatio(ratio);
}

//float XPR_MCVR_GetAspectRatio()
float XPR_MCVR_GetAspectRatio()
{
    CHECK_RENDERER_RETVAL(0.0f);
    return _mcvr_renderer->getAspectRatio();
}

//void XPR_MCVR_SetScale(int port, float factor)
void XPR_MCVR_SetScale(int port, float factor)
{
    CHECK_RENDERER_NORET();
    _mcvr_renderer->setScale(port, factor);
}

//float XPR_MCVR_GetScale(int port)
float XPR_MCVR_GetScale(int port)
{
    CHECK_RENDERER_RETVAL(0.0f);
    return _mcvr_renderer->getScale(port);
}

int XPR_MCVR_SetRate(int port, float rate)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->setRate(port, rate) ? 1 : 0;
}

int XPR_MCVR_SetRenderLevel(XPR_MCVR_RenderLevel level)
{
    CHECK_RENDERER_RETVAL(XPR_MCVR_RENDER_LEVEL_0);
    return _mcvr_renderer->setRenderLevel(level) ? 1 : 0;
}

XPR_MCVR_RenderLevel XPR_MCVR_GetRenderLevel(void)
{
    CHECK_RENDERER_RETVAL(XPR_MCVR_RENDER_LEVEL_0);
    return static_cast<XPR_MCVR_RenderLevel>(_mcvr_renderer->getRenderLevel());
}

int XPR_MCVR_SetString(int port, XPR_MCVR_StringType type, char *strings)
{
    CHECK_RENDERER_RETVAL(0);
    if(XPR_MCVR_STRING_TYPE_TITLE == type) {
        return _mcvr_renderer->setChannelString(port, type, strings);
    }

    return 0;
}

int XPR_MCVR_SetStrings(int port, XPR_MCVR_StringType type, char **strings, int count)
{
    CHECK_RENDERER_RETVAL(0);
    if(XPR_MCVR_STRING_TYPE_HINT == type) {
        return _mcvr_renderer->setChannelStrings(port, type, strings, count);
    }

    return 0;
}

int XPR_MCVR_SetStringState(int port, XPR_MCVR_StringType type, XPR_MCVR_StringState state)
{
    CHECK_RENDERER_RETVAL(0);

    bool ret = false;
    switch(type) {
        case XPR_MCVR_STRING_TYPE_HINT:
            switch(state) {
            case XPR_MCVR_STRING_STATE_SHOW_1:
                ret = _mcvr_renderer->setChannelState(port, XPR_MCVR_CHANNEL_NO_CAMERA);
                break;
            case XPR_MCVR_STRING_STATE_SHOW_2:
                ret = _mcvr_renderer->setChannelState(port, XPR_MCVR_CHANNEL_BUFFERING);
                break;
            case XPR_MCVR_STRING_STATE_SHOW_3:
                ret = _mcvr_renderer->setChannelState(port, XPR_MCVR_CHANNEL_INTERRUPT);
                break;
            case XPR_MCVR_STRING_STATE_SHOW_4:
                ret = _mcvr_renderer->setChannelState(port, XPR_MCVR_CHANNEL_STOPPED);
                break;
            case XPR_MCVR_STRING_STATE_HIDE:
                ret = _mcvr_renderer->setHintFlag(0);
                break;
            case XPR_MCVR_STRING_STATE_SHOW:
                ret = _mcvr_renderer->setHintFlag(1);
                break;
            default:
                break;
            }
            break;
        case XPR_MCVR_STRING_TYPE_TITLE:
            switch(state) {
            case XPR_MCVR_STRING_STATE_HIDE:
                ret = _mcvr_renderer->setTitleFlag(0);
                break;
            case XPR_MCVR_STRING_STATE_SHOW:
                ret = _mcvr_renderer->setTitleFlag(1);
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }

    return ret ? 1 : 0;
}

XPR_MCVR_StringState XPR_MCVR_GetStringState(int port, XPR_MCVR_StringType type)
{
    CHECK_RENDERER_RETVAL(XPR_MCVR_STRING_STATE_HIDE);
    switch(type) {
    case XPR_MCVR_STRING_TYPE_TITLE:
        return 0<_mcvr_renderer->getTitleFlag() ? XPR_MCVR_STRING_STATE_SHOW : XPR_MCVR_STRING_STATE_HIDE;
    case XPR_MCVR_STRING_TYPE_HINT:
        {
            if(0 < _mcvr_renderer->getHintFlag()) {
                switch(_mcvr_renderer->getChannelState(port)) {
                case XPR_MCVR_CHANNEL_NO_CAMERA:
                    return XPR_MCVR_STRING_STATE_SHOW_1;
                case XPR_MCVR_CHANNEL_BUFFERING:
                    return XPR_MCVR_STRING_STATE_SHOW_2;
                case XPR_MCVR_CHANNEL_INTERRUPT:
                    return XPR_MCVR_STRING_STATE_SHOW_3;
                case XPR_MCVR_CHANNEL_STOPPED:
                    return XPR_MCVR_STRING_STATE_SHOW_4;
                default:
                    break;
                }
            }
        }
        break;
    default:
        break;
    }

    return XPR_MCVR_STRING_STATE_HIDE;
}

int XPR_MCVR_GetCurrentPort(void)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->getCurrentPort();
}

int XPR_MCVR_SetChannelState(int port, XPR_MCVR_ChannelState state)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->setChannelState(port, state);
}

XPR_MCVR_ChannelState XPR_MCVR_GetChannelState(int port)
{
    CHECK_RENDERER_RETVAL(XPR_MCVR_CHANNEL_NO_CAMERA);
    return _mcvr_renderer->getChannelState(port);
}

int XPR_MCVR_SetEffect(int port, XPR_MCVR_EffectType effect, int value)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->setChannelEffect(port, effect, value);
}

int XPR_MCVR_SetEffectF(int port, XPR_MCVR_EffectType effect, float value)
{
    CHECK_RENDERER_RETVAL(0);
        return _mcvr_renderer->setChannelEffectF(port, effect, value);
}

int XPR_MCVR_GetEffect(int port, XPR_MCVR_EffectType effect)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->getChannelEffect(port, effect);
}

float XPR_MCVR_GetEffectF(int port, XPR_MCVR_EffectType effect)
{
    CHECK_RENDERER_RETVAL(0.0f);
    return _mcvr_renderer->getChannelEffectF(port, effect);
}

int XPR_MCVR_AddEventCallback(XPR_MCVR_EventCallback callback, void *user)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->addEventCallback(callback, user) ? 1 : 0;
}

int XPR_MCVR_DelEventCallback(XPR_MCVR_EventCallback callback, void *user)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->delEventCallback(callback, user) ? 1 : 0;
}

int XPR_MCVR_AttachEvent(XPR_MCVR_EventType ev)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->attachEvent(ev) ? 1 : 0;
}

int XPR_MCVR_AttachAllEvents(void)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->attachAllEvents() ? 1 : 0;
}

int XPR_MCVR_DetachEvent(XPR_MCVR_EventType ev)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->detachEvent(ev) ? 1 : 0;
}

int XPR_MCVR_DetachAllEvents(void)
{
    CHECK_RENDERER_RETVAL(0);
    return _mcvr_renderer->detachAllEvents() ? 1 : 0;
}
#else
#endif // HAVE_XPR_MCVR_DRIVER_D3D