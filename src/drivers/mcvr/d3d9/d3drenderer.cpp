#include <Windows.h>
#include <d3dx9.h>
#include <Commctrl.h>   //窗口子类化
#include <xpr/xpr_mcvr.h>
#include "../../../resource.h"
#include "d3drenderer.hpp"
#include "d3dchannel.hpp"
#include "safefree.hpp"
#include "singleevent.hpp"
#include "textwriter.hpp"

extern int _last_mcvr_error;

//边框颜色
static const D3DXVECTOR4 kBlackBorder(0.0, 0.0, 0.0, 1.0);      //黑色
static const D3DXVECTOR4 kHighLightBorder(1.0, 1.0, 0.0, 1.0);  //黄色
static const D3DXVECTOR4 kAlarmBorder(1.0, 0.0, 0.0, 1.0);      //红色
//定义优化级别
int _optimize_code = OPTIMIZE_OPERATION_NONE;

//夹角值
static const float SCENE_FOV_Y = D3DX_PI/2.0;
//纹理顶点
#define D3DFVF_VERTEX_TEXTURE (D3DFVF_XYZ | D3DFVF_TEX1)
struct CUSTOMVERTEX_TEXTURE{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 coord[1];
};
//通道状态字符串常量
const char* const STRING_NO_DEVICE      = "无设备";
const char* const STRING_BUFFERING      = "正在缓冲...";
const char* const STRING_INTERRUPTED    = "断开连接...";
const char* const STRING_RENDER_STOPPED = "停止播放";
//警报红色通道增量
const float kAlarmOffset = 0.618f;
//枚举类型代替宏
enum _MCVR_RENDERER_ENUM {
    kSleepThreshold = 10,            //睡眠阈值
    WM_RESET_DEVICE = WM_USER + 139, //重置设备事件
};

namespace XD
{
    // 全局函数
    //=============================================================================
    //图像属性/图像增强强度是否有效
    static inline bool isFactorValid(int val)
    {
        if(val*(100-val) >= 0) {
            return true;
        }
        return false;
    }
    //根据文件路径判断图像格式
    static D3DXIMAGE_FILEFORMAT getImageFormat(const char *path)
    {
        if(nullptr==path) return D3DXIFF_FORCE_DWORD;
        const char*temp_str = strrchr(path, '.') +1;
        if(nullptr==temp_str) return D3DXIFF_FORCE_DWORD;
        if(0==strcmp("png", temp_str) || 0==strcmp("PNG", temp_str)) {
            return D3DXIFF_PNG;
        }
        else if(0==strcmp("jpg", temp_str) || 0==strcmp("JPG", temp_str)) {
            return D3DXIFF_JPG;
        }
        else if(0==strcmp("bmp", temp_str) || 0==strcmp("BMP", temp_str)) {
            return D3DXIFF_BMP;
        }
        else {
            return D3DXIFF_FORCE_DWORD;
        }
    }

    //声明窗口子类化函数
    LRESULT __stdcall SubClassProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData);
    LRESULT __stdcall SubClassProcEx(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData);
    // D3DRenderer
    //=============================================================================
    D3DRenderer::D3DRenderer(void *hwnd)
        :_device(nullptr)
        ,_main_hwnd(reinterpret_cast<HWND>(hwnd))
        ,_wnd_size(0)
        ,_center_vertex(nullptr)
        ,_full_vertex(nullptr)
        ,_d3d_effect(nullptr)
        ,_aspect_ratio(0.0f)
        ,_callback_lock()
        ,_event_lock()
        ,_pixel_table(nullptr)
        ,_buffer_count(kMaxBufferCount)
        ,_end_event(nullptr)
        ,_previous_time(0)
        ,_text_writer(nullptr)
        ,_d3d_device_state(D3D_DEVICE_STATE_NORMAL)
        ,_camera_state_string(nullptr)
        ,_render_level(XPR_MCVR_RENDER_LEVEL_0)
        ,_is_rendering(false)
        ,_is_show_title(true)
        ,_is_show_hint(true)
    {
        bool flag = createDevice(_main_hwnd);
        if(!flag) {
            _last_mcvr_error = XPR_MCVR_ERR_DEVICE_CREATE_FAILED;
            return;
        }
        initStateString();
        flag = initPipeline();
        if(flag) {
            _d3d_effect = new D3DEffect(_device);
            _text_writer = new TextWriter(_device);
            timeBeginPeriod(1);
            beginRenderLoop();
        }
    }

    D3DRenderer::~D3DRenderer()
    {
        _is_rendering = false;
        if(_end_event) {
            _end_event->waitEvent(INFINITE);
            SafeDelete(&_end_event);
        }

        _texture_map.clear();
        for(int i=0; i<=kMaxChannelCount; ++i) {
            removeMainChannel(i);
        }

        SafeDelete(&_camera_state_string);
        SafeDelete(&_text_writer);
        SafeRelease(&_full_vertex);
        SafeRelease(&_center_vertex);
        SafeRelease(&_pixel_table);
        SafeDelete(&_d3d_effect);
        SafeRelease(&_device);
        //_event_lock.Lock();
        _event_set.clear();
        //_event_lock.Unlock();
        //_callback_lock.Lock();
        _callback_set.clear();
        //_callback_lock.Unlock();
        RemoveWindowSubclass(_main_hwnd, SubClassProc, reinterpret_cast<UINT_PTR>(this));
        timeEndPeriod(1);
    }

    bool D3DRenderer::createDevice(HWND hwnd)
    {
        HRESULT hr = S_OK;
        CComPtr<IDirect3D9> d3d9;
        d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
        if(!d3d9) {
            return false;
        }

        if(!hwnd) {
            hwnd = GetDesktopWindow();
            _main_hwnd = hwnd;
        }
        //使用桌面窗口宽高，减少缩放锯齿
        RECT rect;
        GetWindowRect(hwnd, &rect);
        UINT width = rect.right - rect.left;
        UINT height = rect.bottom - rect.top;
        _wnd_size = (static_cast<DWORD>(height&0xFFFF) << 16);
        _wnd_size += static_cast<DWORD>(width&0xFFFF);

        D3DPRESENT_PARAMETERS pp;
        fillPresentParameters(&pp, hwnd, width, height);
        // Get the device caps for this adapter.
        D3DCAPS9 caps;
        memset(&caps, 0, sizeof(caps));
        hr = d3d9->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
        if(FAILED(hr)) {
            return false;
        }
        DWORD vp = 0;
        if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
            vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        } else {
            vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
        }

        // Create device.
        hr = d3d9->CreateDevice(
            0, D3DDEVTYPE_HAL, pp.hDeviceWindow,
            vp | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
            &pp, &_device);
        if(FAILED(hr)) {
            return false;
        }

        //窗口子类化
        SetWindowSubclass(
            _main_hwnd, SubClassProc,
            reinterpret_cast<UINT_PTR>(this),
            reinterpret_cast<DWORD_PTR>(this));

        return true;
    }

    void D3DRenderer::fillPresentParameters(D3DPRESENT_PARAMETERS *pp, HWND hwnd, UINT width, UINT height)
    {
        if(!pp) {
            return;
        }

        memset(pp, 0, sizeof(*pp));
        pp->BackBufferWidth = width;
        pp->BackBufferHeight = height;
        pp->Windowed = TRUE;
        pp->SwapEffect = D3DSWAPEFFECT_FLIP;
        pp->BackBufferFormat = D3DFMT_UNKNOWN;
        pp->BackBufferCount = 1;
        pp->hDeviceWindow = hwnd;
        pp->MultiSampleType = D3DMULTISAMPLE_NONE;
        pp->MultiSampleQuality = 0;
        pp->EnableAutoDepthStencil = FALSE;
        pp->FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        pp->Flags = D3DPRESENTFLAG_VIDEO;
        pp->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }

    bool D3DRenderer::initPipeline(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }

        HRESULT hr = S_OK;
        //设置变换矩阵
        D3DXMATRIXA16 matView, matProj;
        D3DXMatrixIdentity( &matView );
        D3DXVECTOR3 vEyePt   (0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 2.0f);
        D3DXVECTOR3 vUpVec   (0.0f, 1.0f, 0.0f);
        D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
        _device->SetTransform(D3DTS_VIEW, &matView);
        D3DXMatrixPerspectiveFovLH(&matProj, SCENE_FOV_Y, 16.0f/9.0f, 0.1f, 10.0f);
        hr = _device->SetTransform(D3DTS_PROJECTION, &matProj);
        //设置渲染状态
        _device->SetRenderState(D3DRS_CULLMODE,         D3DCULL_NONE);
        _device->SetRenderState(D3DRS_LIGHTING,         FALSE);
        _device->SetRenderState(D3DRS_ZENABLE,          D3DZB_FALSE);
        _device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        _device->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
        _device->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
        //设置采样状态
        for(int i=0; i<3; ++i) {
            _device->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            _device->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            _device->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
            _device->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
            _device->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
        }
        //创建像素映射表
        hr = _device->CreateTexture(
            256, 1, kTextureLevel, D3DUSAGE_DYNAMIC, D3DFMT_L8,
            D3DPOOL_DEFAULT, &_pixel_table, NULL);
        return  initVertices();
    }

    bool D3DRenderer::initVertices(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }

        //Full region vertices data.
        CUSTOMVERTEX_TEXTURE full_vertex[4];
        full_vertex[0].pos = D3DXVECTOR3(-8.0f, -4.5f, 4.5f);
        full_vertex[1].pos = D3DXVECTOR3(-8.0f, 4.5f, 4.5f);
        full_vertex[2].pos = D3DXVECTOR3(8.0f, -4.5f, 4.5f);
        full_vertex[3].pos = D3DXVECTOR3(8.0f, 4.5f, 4.5f);
        for(int i=0; i<1; ++i) {
            full_vertex[0].coord[i] = D3DXVECTOR2(0.0f, 1.0f);  //左下
            full_vertex[1].coord[i] = D3DXVECTOR2(0.0f, 0.0f);  //左上
            full_vertex[2].coord[i] = D3DXVECTOR2(1.0f, 1.0f);  //右下
            full_vertex[3].coord[i] = D3DXVECTOR2(1.0f, 0.0f);  //右上
        }

        //Center region vertices data.
        float golden_ratio = 0.618f;
        CUSTOMVERTEX_TEXTURE center_vertex[4];
        for(int i=0; i<4; ++i) {
            center_vertex[i].pos.x = full_vertex[i].pos.x * golden_ratio;
            center_vertex[i].pos.y = full_vertex[i].pos.y * golden_ratio;
            center_vertex[i].pos.z = full_vertex[i].pos.z;
            for(int j=0; j<1; ++j) {
                center_vertex[i].coord[j] = full_vertex[i].coord[j];
            }
        }

        UINT vertex_len = sizeof(CUSTOMVERTEX_TEXTURE) * 4;
        //Full vertex buffer.
        HRESULT hr = _device->CreateVertexBuffer(
            vertex_len, D3DUSAGE_WRITEONLY,
            D3DFVF_VERTEX_TEXTURE, D3DPOOL_DEFAULT,
            &_full_vertex, nullptr);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            return false;
        }
        void *data = nullptr;
        hr = _full_vertex->Lock(0, 0, &data, D3DLOCK_NOSYSLOCK);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            return false;
        }
        memcpy_s(data, vertex_len, full_vertex, vertex_len);
        _full_vertex->Unlock();
        //Center vertex buffer.
        hr = _device->CreateVertexBuffer(
            vertex_len, D3DUSAGE_WRITEONLY,
            D3DFVF_VERTEX_TEXTURE, D3DPOOL_DEFAULT,
            &_center_vertex, nullptr);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            return false;
        }
        data = nullptr;
        hr = _center_vertex->Lock(0, 0, &data, D3DLOCK_NOSYSLOCK);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            return false;
        }
        memcpy_s(data, vertex_len, center_vertex, vertex_len);
        _center_vertex->Unlock();

        _device->SetFVF(D3DFVF_VERTEX_TEXTURE);
        return true;
    }

    bool D3DRenderer::resetDevice(int wnd_size)
    {
        if(!_device || 0>=wnd_size) {
            _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
            _d3d_device_state = D3D_DEVICE_STATE_NORMAL;
            return false;
        }

        //Release textures.
        stdext::hash_map<UINT, std::shared_ptr<D3DDestTexture>>::iterator tex_it;
        std::vector<UINT> tex_params;
        for(tex_it=_texture_map.begin(); tex_it!=_texture_map.end(); ++tex_it) {
            tex_params.push_back(tex_it->first);
        }
        _texture_map.clear();
        SafeRelease(&_pixel_table);
        //Releae vertices.
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator main_it;
        std::shared_ptr<D3DMainChannel> main_channel;
        for(main_it=_channels_map.begin(); main_it!=_channels_map.end(); ++main_it) {
            main_channel = main_it->second;
            if(main_channel.get()) {
                main_channel->onLostDevice();
            }
            main_channel.reset();
        }
        main_it = _channels_map.end();
        //Releaset pipeline
        SafeRelease(&_center_vertex);
        SafeRelease(&_full_vertex);
        if(_d3d_effect) {
            _d3d_effect->onLostDevice();
        }
        if(_text_writer) {
            _text_writer->onLostDevice();
        }

        //Reset Direct3D device.
        UINT width = static_cast<UINT>(wnd_size & 0xFFFF);
        UINT height = static_cast<UINT>((wnd_size>>16) & 0xFFFF);
        D3DPRESENT_PARAMETERS pp;
        fillPresentParameters(&pp, _main_hwnd, width, height);
        HRESULT hr = _device->Reset(&pp);

        //Create textures.
        UINT params = 0;
        while(!tex_params.empty()) {
            params = tex_params.back();
            tex_params.pop_back();
            if(!getDestTexture(params).get()) {
                //WPRINT_DEBUG_MSG(L"Failed to create dest textures.");
            }
        }
        //Create vetices.
        for(main_it=_channels_map.begin(); main_it!=_channels_map.end(); ++main_it) {
            main_channel = main_it->second;
            if(main_channel.get()) {
                main_channel->onResetDevice();
            }
            main_channel.reset();
        }
        main_it = _channels_map.end();
        //Create effect and text writer.
        initPipeline();
        if(_d3d_effect) {
            _d3d_effect->onResetDevice();
        }
        if(_text_writer) {
            _text_writer->onResetDevice();
        }

        _d3d_device_state = D3D_DEVICE_STATE_NORMAL;
        return true;
    }

    void D3DRenderer::initStateString()
    {
        SafeDelete(&_camera_state_string);
        _camera_state_string = new XPR_MCVR_StateString();
        if(_camera_state_string) {
            int len = strlen(STRING_NO_DEVICE) + 1;
            memcpy_s(_camera_state_string->no_device_string, len, STRING_NO_DEVICE, len);
            len = strlen(STRING_BUFFERING) + 1;
            memcpy_s(_camera_state_string->buffering_string, len, STRING_BUFFERING, len);
            len = strlen(STRING_RENDER_STOPPED) + 1;
            memcpy_s(_camera_state_string->stopped_string, len, STRING_RENDER_STOPPED, len);
            len = strlen(STRING_INTERRUPTED) + 1;
            memcpy_s(_camera_state_string->interrupt_string, len, STRING_INTERRUPTED, len);
        }
    }

    bool D3DRenderer::bindPort(int port, void *hwnd, int rows, int cols)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }
        if(!isPortValid(port)) {
            _last_mcvr_error = XPR_MCVR_ERR_INVALID_PORT;
            return false;
        }

        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        std::shared_ptr<D3DMainChannel> main_channel;
        map_it = _channels_map.find(port);

        if(_channels_map.end()==map_it || !map_it->second.get()) {    //新建主通道
            if(!hwnd) {
                _channels_map.erase(map_it);
                return true;
            }
            if(!isPortValid(rows*cols)) {
                _last_mcvr_error = XPR_MCVR_ERR_INVALID_LAYOUT;
                return false;
            }
            if(_channels_map.end() != map_it) { //删除空主通道
                removeMainChannel(map_it->first);
            }
            return addMainChannel(port, reinterpret_cast<HWND>(hwnd), rows, cols);
        }
        else {
            if(!hwnd) { //删除该主通道
                removeMainChannel(port);
                return true;
            }
            main_channel = map_it->second;
            if(main_channel->getHwnd() == reinterpret_cast<HWND>(hwnd)) {    //重新布局该主通道
                MAIN_CHANNEL_MISSION mission;
                mission.type = MAIN_CHANNEL_LAYOUT;
                mission.data = (rows<<16) | (cols&0xFFFF);
                main_channel->postMission(&mission);
                return true;
            }
            if(isPortValid(rows * cols)) {  //将另一窗口绑定到该主通道
                MAIN_CHANNEL_MISSION mission;
                mission.type = MAIN_CHANNEL_BINDING;
                mission.data = reinterpret_cast<int>(hwnd);
                main_channel->postMission(&mission);
                mission.type = MAIN_CHANNEL_LAYOUT;
                mission.data = (rows<<16) | (cols&0xFFFF);
                main_channel->postMission(&mission);
            }
            _last_mcvr_error = XPR_MCVR_ERR_INVALID_LAYOUT;
            return false;
        }

        _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
        return false;
    }

    bool D3DRenderer::addMainChannel(int main_port, HWND hwnd, int rows, int cols)
    {
        if(isPortValid(main_port)) {
            std::shared_ptr<D3DMainChannel> main_channel;
            main_channel.reset(new D3DMainChannel(_device, hwnd, rows, cols, _buffer_count));
            if(main_channel.get()) {
                _channels_map.insert(std::pair<int, std::shared_ptr<D3DMainChannel>>(main_port, main_channel));
                main_channel->createLinePainter(_main_hwnd);
                SetWindowSubclass(hwnd, SubClassProcEx, static_cast<UINT_PTR>(main_port), reinterpret_cast<DWORD_PTR>(this));
                return true;
            }
        }

        _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
        return false;
    }

    void D3DRenderer::removeMainChannel(int main_port)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(main_port);
        if(main_channel.get()) {
            RemoveWindowSubclass(main_channel->getHwnd(), SubClassProcEx, static_cast<UINT_PTR>(main_port));
            int src_port = main_channel->getSourcePort();
            main_channel.reset();
            main_channel = getMainChannel(src_port>>16);
            if(main_channel.get()) {
                main_channel->setZoomPort(src_port&0xFFFF, -1);
            }
            main_channel.reset();
        }
        _channels_map.erase(main_port);
    }

    LRESULT __stdcall SubClassProc(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData)
    {
        D3DRenderer *d3d_renderer = reinterpret_cast<D3DRenderer*>(dwRefData);
        if(d3d_renderer) {
            return d3d_renderer->respondMainHwnd(hWnd, uMsg, wParam, lParam);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    LRESULT __stdcall SubClassProcEx(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        UINT_PTR uIdSubclass,
        DWORD_PTR dwRefData)
    {
        D3DRenderer *d3d_renderer = reinterpret_cast<D3DRenderer*>(dwRefData);
        if(d3d_renderer) {
            return d3d_renderer->respondChannelHwnd(hWnd, uMsg, wParam, lParam, uIdSubclass);
        }

        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    LRESULT D3DRenderer::respondMainHwnd(HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT ret = 0;
        switch(uMsg) {
        case WM_SIZE:
            {
                ret = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                if(D3D_DEVICE_STATE_NORMAL == _d3d_device_state) {
                    _d3d_device_state = D3D_DEVICE_STATE_INVALID;
                    _wnd_size = lParam;
                }
                return ret;
            }
        case WM_RESET_DEVICE:
            {
                resetDevice(lParam);
                return 0;
            }
        case WM_CLOSE:
            RemoveWindowSubclass(hWnd, SubClassProc, reinterpret_cast<UINT_PTR>(this));
            _is_rendering = false;
            break;
        case WM_PAINT:
            break;
        default:
            break;
        }

        ret = DefSubclassProc(hWnd, uMsg, wParam, lParam);
        return ret;
    }

    LRESULT D3DRenderer::respondChannelHwnd(HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR id)
    {
        LRESULT ret = 0;
        switch(uMsg) {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            {
                ret = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                resetCurrentPort();
                int main_port = static_cast<int>(id);
                std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(main_port);
                if(main_channel.get()) {
                    int sub_port = main_channel->clickOnWnd(lParam);
                    int source_port = main_channel->getSourcePort();
                    if(0 == source_port) {
                        _current_port = (main_port<<16) | (sub_port&0xFFFF);
                    }
                    else if(isPortValid(source_port>>16)) {
                        _current_port = source_port & 0xFFFF0000;
                    }
                    else {
                        _current_port = (main_port<<16) | (sub_port&0xFFFF);
                    }
                    MAIN_CHANNEL_MISSION mission;
                    mission.type = MAIN_CHANNEL_CHOOSE;
                    mission.data = sub_port;
                    main_channel->postMission(&mission);
                }

                XPR_MCVR_EventType event = XPR_MCVR_LEFT_CLICK;
                if(WM_RBUTTONDOWN == uMsg) {
                    event = XPR_MCVR_RIGHT_CLICK;
                }
                int pos = static_cast<int>(lParam);
                genericEvent(event, _current_port, reinterpret_cast<void*>(&pos));
            }
            return ret;
        case WM_LBUTTONDBLCLK:
            {
                ret = DefSubclassProc(hWnd, uMsg, wParam, lParam);
                int pos = static_cast<int>(lParam);
                genericEvent(XPR_MCVR_LEFT_DBCLICK, _current_port, reinterpret_cast<void*>(&pos));
            }
            return ret;
        case WM_SIZE:
            ret = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if(hWnd != _main_hwnd) {
                int main_port = static_cast<int>(id);
                std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(main_port);
                if(main_channel.get()) {
                    MAIN_CHANNEL_MISSION mission;
                    mission.type = MAIN_CHANNEL_RESIZE;
                    mission.data = lParam;
                    main_channel->postMission(&mission);
                }
            }
            return ret;
        case WM_CLOSE:
            if(hWnd != _main_hwnd) {
                RemoveWindowSubclass(hWnd, SubClassProcEx, id);
                int port = static_cast<int>(id);
                _channels_map.erase(id);
            }
            break;
        default:
            break;
        }
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }

    bool D3DRenderer::inputData(int port, XPR_AVFrame *av_frame)
    {
        if(!_device) {
            return false;
        }

        /*std::shared_ptr<D3DSubChannel> sub_channel;
        sub_channel = getSubChannel(port);
        if(sub_channel.get()) {
            return sub_channel->inputData(av_frame);
        }*/
        std::shared_ptr<D3DMainChannel> main_channel;
        main_channel = getMainChannel((port>>16)&0x0000FFFF);
        if(main_channel.get()) {
            return main_channel->inputData(port&0x0000FFFF, av_frame);
        }

        return false;
    }

    //dest_port:
    //-1     取消放大模式
    //0      本窗口全区域放大
    //0xFFFF 本窗口中心区域放大
    //1~256  其他窗口全区域放大
    bool D3DRenderer::zoomIn(int src_port, int dest_port)
    {
        //判断目标通道是否有效
        if(-1!=dest_port && 0x0000!=dest_port && 0xFFFF!=dest_port && !isMainChanValid(dest_port)) {
            _last_mcvr_error = XPR_MCVR_ERR_INVALID_PORT;
            return false;
        }
        
        int main_port = src_port >> 16;
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(main_port);
        if(!main_channel.get()) {
            return false;
        }
        //避免重复设置
        int cur_port = main_channel->getZoomPort(src_port&0xFFFF);
        if(dest_port == cur_port) {
            return true;
        }
        //避免目标通道号对于本主通道号
        if(main_port == dest_port) {
            dest_port = 0x0000;
        }
        //重置原放大目标窗口的源通道号
        std::shared_ptr<D3DMainChannel> zoom_channel;
        if(0 == cur_port) {
            main_channel->setSourcePort(0x0000);
        } else if(isPortValid(cur_port)) {
            zoom_channel = getMainChannel(cur_port);
            if(zoom_channel.get()) {
                zoom_channel->setSourcePort(0x0000);
            }
            zoom_channel.reset();
        }
        //根据放大模式执行不同操作
        if(-1 == dest_port) {           //取消放大
            main_channel->setZoomPort(src_port&0xFFFF, -1);
        }
        else if(0x0000 == dest_port) {  //本窗口全区域放大
            main_channel->setZoomPort(src_port&0xFFFF, 0);
            if(main_channel.get()) {
                main_channel->setSourcePort(src_port & 0x0000FFFF);
            }
        }
        else if(0xFFFF == dest_port) {  //本窗口中心区域放大
            main_channel->setZoomPort(src_port&0xFFFF, 0xFFFF);
            if(main_channel.get()) {
                main_channel->setSourcePort(src_port & 0xFFFF);
            }
        }
        else {                          //其他窗口全区域放大
            main_channel->setZoomPort(src_port&0xFFFF, dest_port);
            main_channel.reset();
            main_channel = getMainChannel(dest_port);
            if(main_channel.get()) {
                main_channel->setSourcePort(src_port);
            }
        }

        return true;
    }

    int D3DRenderer::getZoomPort(int src_port)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(src_port>>16);
        if(main_channel.get()) {
            return main_channel->getZoomPort(src_port&0xFFFF);
        }

        return -1;
    }

    bool D3DRenderer::resetPort(int port)
    {
        int main_port = (port>>16) & 0xFFFF;
        int sub_port = port & 0xFFFF;

        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        std::shared_ptr<D3DMainChannel> main_channel;
        if(0 == main_port) {
            for(map_it=_channels_map.begin(); map_it!=_channels_map.end(); ++map_it) {
                main_channel = map_it->second;
                if(main_channel.get()) {
                    main_channel->resetSubChannels(0);
                }
                main_channel.reset();
            }
        }
        else if(isPortValid(main_port)) {
            main_channel = getMainChannel(main_port);
            if(main_channel.get()) {
                main_channel->resetSubChannels(sub_port);
            }
        }

        return true;
    }

    int D3DRenderer::getCurrentPort()
    {
        return _current_port;
    }

    void D3DRenderer::resetCurrentPort()
    {
        std::shared_ptr<D3DMainChannel> main_channel;
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it = _channels_map.begin();
        for(map_it=_channels_map.begin(); map_it!=_channels_map.end(); ++map_it) {
            main_channel = map_it->second;
            if(main_channel.get()) {
                MAIN_CHANNEL_MISSION mission;
                mission.type = MAIN_CHANNEL_CHOOSE;
                mission.data = 0;
                main_channel->postMission(&mission);
            }
            main_channel.reset();
        }
    }
    
    bool D3DRenderer::getVideoSize(int port, int *width, int *height)
    {
        if(width && height) {
            std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(port);
            if(sub_channel.get()) {
                *width = sub_channel->getWidth();
                *height = sub_channel->getHeight();
                return true;
            }
        }

        return false;
    }

    bool D3DRenderer::snapShot(int port, const char *path)
    {
        std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(port);
        if(sub_channel.get()) {
            sub_channel->takeSnapshot(path);
            return true;
        }

        return false;
    }

    void D3DRenderer::setAspectRatio(float ratio)
    {
        _aspect_ratio = ratio;
    }

    float D3DRenderer::getAspectRatio()
    {
        return _aspect_ratio;
    }

    void D3DRenderer::setScale(int port, float factor)
    {
        return;
    }

    float D3DRenderer::getScale(int port)
    {
        return 0.0f;
    }
    
    bool D3DRenderer::setRate(int port, float rate)
    {
        std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(port);
        if(sub_channel.get()) {
            sub_channel->setRate(rate);
            return true;
        }

        return false;
    }

    UINT __stdcall loopThreadCallback(void *param)
    {
        D3DRenderer *renderer = reinterpret_cast<D3DRenderer*>(param);
        if(renderer) {
            renderer->loopPresenting();
        }
        _endthreadex(0);
        return 0;
    }

    bool D3DRenderer::beginRenderLoop(void)
    {
        HANDLE thread_handle = (HANDLE)_beginthreadex(nullptr, 0, &loopThreadCallback, reinterpret_cast<void*>(this), 0, nullptr);
        _is_rendering = nullptr!=thread_handle ? true : false;

        return _is_rendering;
    }

    void D3DRenderer::loopPresenting(void)
    {
        //创建等待线程结束的事件
        _end_event = new SingleEvent(nullptr);
        if(_end_event) {
            _end_event->resetEvent();
        }

        int count = 0;
        while(_is_rendering) {
            switch(_optimize_code) {
            case OPTIMIZE_OPERATION_NONE:
                break;
            case OPTIMIZE_OPERATION_REDUCING:
                optimizePerformance(false);
                break;
            case OPTIMIZE_OPERATION_IMPROVING:
                optimizePerformance(true);
                break;
            default:
                break;
            }

            switch(_d3d_device_state) {
            case D3D_DEVICE_STATE_NORMAL:
                count = presentAllChannels();
                break;
            case D3D_DEVICE_STATE_INVALID:
                {
                    _d3d_device_state = D3D_DEVICE_STATE_RESETING;
                    PostMessage(_main_hwnd, WM_RESET_DEVICE, 0, _wnd_size);
                }
                break;
            case D3D_DEVICE_STATE_RESETING: //窗口消息响应函数负责重置设备
            default:
                break;
            }

            if(0==count && isFallingSleep()) {
                Sleep(kSleepThreshold);
                _previous_time = timeGetTime();
            }
            count = 0;
        }

        //触发循环结束事件
        if(_end_event) {
            _end_event->setEvent();
        }
    }

    int D3DRenderer::presentAllChannels(void)
    {
        int count = 0;
        std::shared_ptr<D3DMainChannel> main_channel;
        for(int i=1; i<=kMaxChannelCount; ++i) {
            if(!_is_rendering) {
                break;
            }
            main_channel = getMainChannel(i);
            if(main_channel.get()) {
                //TODO:
                HWND hwnd = main_channel->getHwnd();
                main_channel->excuteMissions();
                if(hwnd != main_channel->getHwnd()) {
                    hwnd = main_channel->getHwnd();
                    RemoveWindowSubclass(hwnd, SubClassProcEx, i);
                    SetWindowSubclass(hwnd, SubClassProcEx, static_cast<UINT_PTR>(i), reinterpret_cast<DWORD_PTR>(this));
                }

                count += presentChannel(main_channel.get());
            }
            main_channel.reset();
        }

        return count;
    }

    int D3DRenderer::presentChannel(D3DMainChannel *main_channel)
    {
        if(!main_channel) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
            return 0;
        }
        if(XPR_MCVR_RENDER_LEVEL_0<_render_level && !main_channel->waitData(kSleepThreshold)) {
            return 0;
        }

        HWND hwnd = main_channel->getHwnd();
        int src_port = main_channel->getSourcePort();
        int main_port = (src_port&0xFFFF0000) >> 16;
        int sub_port = (src_port&0x0000FFFF);

        HRESULT hr = S_OK;
        int count = 0;
        CComPtr<IDirect3DSurface9> orig_surface;
        hr = _device->BeginScene();
        _device->GetRenderTarget(0, &orig_surface);
        setRenderTarget(main_channel->getTargetTexture());
        int channels = main_channel->getChannelsCount();
        //正常显示/中心窗口放大
        if(0==src_port || (0xFFFF==main_port&&isPortValid(sub_port))) {
            //绘制子通道区域
            for(int i=1; i<channels+1; ++i) {
                if(!_is_rendering) {
                    _device->SetRenderTarget(0, orig_surface);
                    hr = _device->EndScene();
                    hr = _device->Present(nullptr, nullptr, hwnd, nullptr);
                    return count;
                }
                if(i!=sub_port && drawChannel(main_channel, i)) {
                    ++count;
                }
            }
            // 绘制中心区域
            if(0xFFFF==main_port && drawChannel(main_channel, sub_port)) {
                ++count;
            }
        }
        //内子通道全窗口显示
        else if(0==main_port && isPortValid(sub_port)) {
            count += drawChannel(main_channel, sub_port) ? 1 : 0;
        }
        //外子通道全窗口显示
        else if(isSubChanValid(src_port)) {
            ++count;
        }

        //避免边框线条显示异常
        hr = _device->SetTexture(0, NULL);
        hr = _device->SetTexture(1, NULL);  
        hr = _device->SetTexture(2, NULL);
        drawText(main_channel);
        if(_render_level < XPR_MCVR_RENDER_LEVEL_4) {
            main_channel->drawLines();
        }
        //将渲染目标设置回后备缓冲，并交换缓冲区
        _device->SetRenderTarget(0, orig_surface);
        orig_surface = NULL;
        setFullVertex();
        hr = _device->SetTexture(0, main_channel->getTargetTexture());
        RECT rect;
        GetClientRect(hwnd, &rect);
        _d3d_effect->setOffset(rect.right-rect.left, rect.bottom-rect.top);
        _d3d_effect->drawRGB();
        _device->EndScene();

        //仅在边框绘制未被忽略或有画面更新时交换缓冲区
        if(_render_level<XPR_MCVR_RENDER_LEVEL_3 || 0<count) {
            _device->Present(NULL, NULL, hwnd, NULL);
        }
        return count;
    }

    bool D3DRenderer::drawChannel(D3DMainChannel *main_channel, const int sub_port)
    {
        if(!_device || !_d3d_effect || !main_channel) {
            _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
            return false;
        }
        //当前子通道数据是否可用
        std::shared_ptr<D3DSubChannel> sub_channel;
        sub_channel = main_channel->getSubChannel(sub_port);
        if(!sub_channel.get()) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
            return false;
        }
        XPR_AVPixelFormat format = sub_channel->getFormat();
        if(XPR_AV_PIX_FMT_YUV420P != format) {
            _last_mcvr_error = XPR_MCVR_ERR_INVALID_PIXEL_FORMAT;
            return false;
        }

        //更新纹理
        bool ret = true;
        UINT params = 0;
        params = sub_channel->getParams();
        std::shared_ptr<D3DDestTexture> dest_texture = getDestTexture(params);
        ret = sub_channel->updateDestTexture(dest_texture.get());
        if(!ret) {
            return false;
        }
        _d3d_effect->setOffset(sub_channel->getWidth(), sub_channel->getHeight());
        //截图
        if(sub_channel->getSnapShotFlag())
        {
            saveTextureToFile(dest_texture->target_tex, sub_channel->getSnapshotPath());
            sub_channel->resetSnapshotFlag();
        }
        //Y通道图像增强
        if(main_channel->isEnhancing(sub_port)) {
            ret = enhanceImageY(main_channel, sub_port, dest_texture.get());
            if(!ret) {
                _last_mcvr_error = XPR_MCVR_ERR_IMAGE_ENHANCE_FAILED;
                return ret;
            }
        }

        //根据放大目标通道号执行不同绘图操作
        ret = main_channel->setRegionSource(sub_port);
        int zoom_port = main_channel->getZoomPort(sub_port);
        bool is_alarming = main_channel->isAlarming(sub_port);
        if(0 > zoom_port) {                     //当前子通道
            ret = main_channel->setRegionSource(sub_port);
            adjustAspectRatio(main_channel->getRegionRatio(), sub_channel->getAspectRatio());
            ret = _d3d_effect->convertColor(format, is_alarming);
        }
        else if(0 == zoom_port) {               //当前主通道全窗口
            ret = setFullVertex();
            adjustAspectRatio(main_channel->getZoomRatio(), sub_channel->getAspectRatio());
            ret = _d3d_effect->convertColor(format, is_alarming);
        }
        else if(0xFFFF == zoom_port) {          //当前主通道中心窗口
            //绘制当前区域
            ret = main_channel->setRegionSource(sub_port);
            adjustAspectRatio(main_channel->getRegionRatio(), sub_channel->getAspectRatio());
            ret = _d3d_effect->convertColor(format, is_alarming);
            //绘制中心区域
            ret = setCenterVertex();
            ret = _d3d_effect->setRatio(main_channel->getZoomRatio(), sub_channel->getAspectRatio());
            ret = _d3d_effect->convertColor(format, is_alarming);
        }
        else if(isMainChanValid(zoom_port)) {   //其他主通道全窗口
            std::shared_ptr<D3DMainChannel> zoom_channel = getMainChannel(zoom_port);
            if(zoom_channel.get() && zoom_channel->getTargetTexture()) {
                //绘制放大窗口
                setFullVertex();
                CComPtr<IDirect3DSurface9> orig_surface;
                _device->GetRenderTarget(0, &orig_surface);
                ret = setRenderTarget(zoom_channel->getTargetTexture());
                adjustAspectRatio(zoom_channel->getZoomRatio(), sub_channel->getAspectRatio());
                ret = _d3d_effect->convertColor(format, is_alarming);
                //绘制当前区域
                _device->SetRenderTarget(0, orig_surface);
                _device->SetTexture(0, zoom_channel->getTargetTexture());
                _device->SetRenderTarget(0, orig_surface);
                ret = main_channel->setRegionSource(sub_port);
                adjustAspectRatio(main_channel->getRegionRatio(), sub_channel->getAspectRatio());
                _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
                //通知放大窗口画面已更新
                zoom_channel->postData();
            } else {
                //绘制当前区域
                ret = main_channel->setRegionSource(sub_port);
                adjustAspectRatio(main_channel->getRegionRatio(), sub_channel->getAspectRatio());
                ret = _d3d_effect->convertColor(format, is_alarming);
            }
        } else {
            _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
            return false;
        }
        
        _device->SetTexture(0, NULL);
        _device->SetTexture(1, NULL);  
        _device->SetTexture(2, NULL);
        return ret;
    }

    void D3DRenderer::saveTextureToFile(IDirect3DTexture9 *texture, const char *path)
    {
        if(!texture || !path) {
            return;
        }

        CComPtr<IDirect3DSurface9> ori_surface;
        _device->GetRenderTarget(0, &ori_surface);
        setFullVertex();
        setRenderTarget(texture);
        adjustAspectRatio(1.0f, 1.0f);
        _d3d_effect->convertColor(XPR_AV_PIX_FMT_YUV420P, false);
        _device->SetRenderTarget(0, ori_surface);
        D3DXIMAGE_FILEFORMAT file_format = getImageFormat(path);
        switch(file_format) {
        case D3DXIFF_PNG:
        case D3DXIFF_JPG:
        case D3DXIFF_BMP:
            D3DXSaveTextureToFileA(path, file_format, texture, nullptr);
            break;
        default:
            break;
        }
    }

    void D3DRenderer::drawText(D3DMainChannel *main_channel)
    {
        if(!main_channel || !_text_writer) {
            return;
        }

        int src_port = main_channel->getSourcePort();
        int main_port = (src_port&0xFFFF0000) >> 16;
        int sub_port = (src_port&0x0000FFFF);
        if(0x00000000 == src_port) {    //正常渲染
            int channels = main_channel->getChannelsCount();
            for(int i=1; i<channels+1; ++i) {
                drawSubText(
                    main_channel->getChannelState(i),
                    main_channel->getSubRect(i),
                    main_channel->getChannelTitle(i));
            }
        }
        else if(0x0000==main_port && isPortValid(sub_port)) {   //本窗口全区域放大
            drawSubText(
                main_channel->getChannelState(sub_port),
                main_channel->getFullRect(),
                main_channel->getChannelTitle(sub_port));
        }
        else if(isSubChanValid(src_port)) {     //其他窗口全区域放大
            std::shared_ptr<D3DMainChannel> src_channel = getMainChannel(main_port);
            if(src_channel.get()) {
                drawSubText(
                    src_channel->getChannelState(sub_port),
                    main_channel->getFullRect(),
                    src_channel->getChannelTitle(sub_port));
            } 
        }
    }

    void D3DRenderer::drawSubText(XPR_MCVR_ChannelState state, RECT rect, char *title)
    {
        if(!_text_writer) {
            return;
        }

        char state_string[20];
        bool is_cleaning = false;;
        switch(state) {
        case XPR_MCVR_CHANNEL_RENDERING:
            break;
        case XPR_MCVR_CHANNEL_NO_CAMERA:
            if(_camera_state_string) {
                sprintf_s(state_string, 20, "%s", _camera_state_string->no_device_string);
                is_cleaning = true;
            }
            break;
        case XPR_MCVR_CHANNEL_BUFFERING:
            if(_camera_state_string) {
                sprintf_s(state_string, 20, "%s", _camera_state_string->buffering_string);
                is_cleaning = true;
            }
            break;
        case XPR_MCVR_CHANNEL_STOPPED:
            if(_camera_state_string) {
                sprintf_s(state_string, 20, "%s", _camera_state_string->stopped_string);
                is_cleaning = true;
            }
            break;
        case XPR_MCVR_CHANNEL_INTERRUPT:
            if(_camera_state_string) {
                sprintf_s(state_string, 20, "%s", _camera_state_string->interrupt_string);
                is_cleaning = true;
            }
            break;
        default:
            if(_camera_state_string) {
                sprintf_s(state_string, 20, "%s", _camera_state_string->no_device_string);
                is_cleaning = true;
            }
            break;
        }
        
        if(is_cleaning && _render_level<XPR_MCVR_RENDER_LEVEL_2 && _device) {
            CComPtr<IDirect3DSurface9> render_target;
            _device->GetRenderTarget(0, &render_target);
            _device->ColorFill(render_target, &rect, D3DCOLOR_XRGB(40, 40, 40));
            _text_writer->drawState(rect, state_string);
        }
        if(_render_level<XPR_MCVR_RENDER_LEVEL_3 && _is_show_title) {
            _text_writer->drawTitle(rect, title);
        }
    }

    bool D3DRenderer::enhanceImageY(D3DMainChannel *main_channel, int sub_port, D3DDestTexture *dest_texture)
    {
        if(!_device || !_d3d_effect || !main_channel || !dest_texture) {
            return false;
        }

        bool ret = setFullVertex();
        if(!ret) return ret;
        int factor;
        CComPtr<IDirect3DSurface9> orig_surface;
        HRESULT hr = _device->GetRenderTarget(0, &orig_surface);
        //Sharpen
        factor = main_channel->getChannelEffect(sub_port, XPR_MCVR_EFFECT_SHARPEN);
        if(0 < factor) {
            setRenderTarget(dest_texture->target_tex);
            ret = _d3d_effect->sharpenImage(factor);
            _device->SetTexture(0, dest_texture->target_tex);
        }
        //Brighten
        factor = main_channel->getChannelEffect(sub_port, XPR_MCVR_EFFECT_BRIGHTEN);
        if(0 < factor) {
            setRenderTarget(dest_texture->target_tex);
            ret = _d3d_effect->brightenImage(factor);
            _device->SetTexture(0, dest_texture->target_tex);
        }
        //Defog
        factor = main_channel->getDefogFactor(sub_port);
        if(0 < factor) {
            std::shared_ptr<D3DSubChannel> sub_channel = main_channel->getSubChannel(sub_port);
            if(sub_channel.get()) {
                switch(main_channel->getDefogMode(sub_port)) {
                case 0:     //Stretch hist defog
                    {
                        float *range = sub_channel->getHistFactor();
                        if(range) {
                            setRenderTarget(dest_texture->target_tex);
                            ret = _d3d_effect->stretchHist(range);
                            _device->SetTexture(0, dest_texture->target_tex);
                        }
                    }
                case 1:     //Equal hist defog
                    {
                        if(sub_channel->updatePixelTable(_pixel_table)) {
                            setRenderTarget(dest_texture->target_tex);
                            ret = _d3d_effect->equalHist();
                            //Set back the texture on stage 0 and 1.
                            _device->SetTexture(0, dest_texture->target_tex);
                            _device->SetTexture(1, dest_texture->u_texture);
                        }
                    }
                default:
                    break;
                }
            }
        }

        _device->SetRenderTarget(0, orig_surface);
        return true;
    }

    std::shared_ptr<D3DMainChannel> D3DRenderer::getMainChannel(int port)
    {
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        map_it = _channels_map.find(port);
        std::shared_ptr<D3DMainChannel> main_channel;
        if(map_it != _channels_map.end()) {
            main_channel = map_it->second;
        }

        if(!main_channel.get()) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        }
        return main_channel;
    }

    std::shared_ptr<D3DSubChannel> D3DRenderer::getSubChannel(int port)
    {
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        map_it = _channels_map.find((port>>16)&0xFFFF);
        std::shared_ptr<D3DSubChannel> sub_channel;
        if(map_it != _channels_map.end()) {
            if(map_it->second.get()) {
                sub_channel = map_it->second->getSubChannel(port&0xFFFF);
            }
            map_it = _channels_map.end();
        }

        if(!sub_channel.get()) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        }
        return sub_channel;
    }

    std::shared_ptr<D3DDestTexture> D3DRenderer::getDestTexture(UINT params)
    {
        std::shared_ptr<D3DDestTexture> d3d_texture;
        if(!_device) {
            return d3d_texture;
        }

        UINT width = static_cast<UINT>((params>>16) & 0xFFFF);
        UINT height = static_cast<UINT>((params>>4) & 0x0FFF);
        UINT bytes = static_cast<UINT>(params & 0x000F);
        if(0 == width*height*bytes) {
            return d3d_texture;
        }

        stdext::hash_map<UINT, std::shared_ptr<D3DDestTexture>>::iterator map_it;
        map_it = _texture_map.find(params);
        if(map_it != _texture_map.end()) {
            return map_it->second;
        }

        d3d_texture.reset(new D3DDestTexture());
        if(!d3d_texture.get()) {
            return d3d_texture;
        }

        HRESULT hr = S_OK;
        switch(bytes) {
        case 1:
            {
                hr = _device->CreateTexture(
                    width, height, kTextureLevel,
                    D3DUSAGE_DYNAMIC,
                    D3DFMT_L8, D3DPOOL_DEFAULT,
                    &d3d_texture->y_texture, nullptr);
                if(SUCCEEDED(hr)) {
                    hr = _device->CreateTexture(
                        width/2, height/2, kTextureLevel,
                        D3DUSAGE_DYNAMIC,
                        D3DFMT_L8, D3DPOOL_DEFAULT,
                        &d3d_texture->u_texture, nullptr);
                }
                if(SUCCEEDED(hr)) {
                    hr = _device->CreateTexture(
                        width/2, height/2, kTextureLevel,
                        D3DUSAGE_DYNAMIC,
                        D3DFMT_L8, D3DPOOL_DEFAULT,
                        &d3d_texture->v_texture, nullptr);
                }
            }
            break;
        case 2:
            hr = _device->CreateTexture(
                width, height, kTextureLevel,
                D3DUSAGE_DYNAMIC,
                D3DFMT_A8P8, D3DPOOL_DEFAULT,
                &d3d_texture->y_texture, nullptr);
            break;
        case 4:
            hr = _device->CreateTexture(
                width, height, kTextureLevel,
                D3DUSAGE_DYNAMIC,
                D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                &d3d_texture->y_texture, nullptr);
            break;
        default:
            return d3d_texture;
        }

        if (SUCCEEDED(hr)) {
            hr = _device->CreateTexture(
                width, height, kTextureLevel, D3DUSAGE_RENDERTARGET,
                D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
                &d3d_texture->target_tex, nullptr);
        }

        if(SUCCEEDED(hr)) {
            _texture_map.insert(std::pair<UINT, std::shared_ptr<D3DDestTexture>>(params, d3d_texture));
        } else {
            _last_mcvr_error = XPR_MCVR_ERR_TEXTURE_CREATE_FAILED;
        }
        return d3d_texture;
    }

    bool D3DRenderer::setCenterVertex(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }

        _device->SetFVF(D3DFVF_VERTEX_TEXTURE);
        HRESULT hr = _device->SetStreamSource(0, _center_vertex, 0, sizeof(CUSTOMVERTEX_TEXTURE));
        return SUCCEEDED(hr) ? true : false;
    }

    bool D3DRenderer::setFullVertex(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }

        _device->SetFVF(D3DFVF_VERTEX_TEXTURE);
        HRESULT hr = _device->SetStreamSource(0, _full_vertex, 0, sizeof(CUSTOMVERTEX_TEXTURE));
        return SUCCEEDED(hr) ? true : false;
    }

    bool D3DRenderer::setRenderTarget(IDirect3DTexture9 *target_tex)
    {
        if(!_device || !target_tex) {
            return false;
        }
        CComPtr<IDirect3DSurface9> cur_surface;
        target_tex->GetSurfaceLevel(0, &cur_surface);
        HRESULT hr = _device->SetRenderTarget(0, cur_surface);
        return true;
    }

    void D3DRenderer::adjustAspectRatio(float wnd_ratio, float video_ratio)
    {
        if(!_d3d_effect) {
            return;
        }

        if(0.000001 > _aspect_ratio) {      //视频原始宽高比
            _d3d_effect->setRatio(wnd_ratio, video_ratio);
        } else if(99.0f < _aspect_ratio) {  //铺满区域
            _d3d_effect->setRatio(wnd_ratio, wnd_ratio);
        } else {                            //自定义宽高比
            _d3d_effect->setRatio(wnd_ratio, _aspect_ratio);
        }
    }

    bool D3DRenderer::setRenderLevel(int level)
    {
        _render_level = level;
        return true;
    }

    int D3DRenderer::getRenderLevel(void) const
    {
        return _render_level;
    }

    bool D3DRenderer::setTitleFlag(int flag)
    {
        _is_show_title = flag>0 ? true : false;
        return true;
    }

    int D3DRenderer::getTitleFlag(void) const
    {
        return _is_show_title ? 1 : 0;
    }

    bool D3DRenderer::setHintFlag(int flag)
    {
        _is_show_hint = flag>0 ? true : false;
        return 1;
    }

    int D3DRenderer::getHintFlag(void) const
    {
        return _is_show_hint ? 1 : 0;
    }

    bool D3DRenderer::setChannelState(int port, XPR_MCVR_ChannelState state)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            main_channel->setChannelState(port&0xFFFF, state);
            return true;
        }

        return false;
    }

    XPR_MCVR_ChannelState D3DRenderer::getChannelState(int port)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            return main_channel->getChannelState(port&0xFFFF);
        }

        return XPR_MCVR_CHANNEL_NO_CAMERA;
    }

    int D3DRenderer::setChannelString(int port, XPR_MCVR_StringType type, char *strings)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(XPR_MCVR_STRING_TYPE_TITLE==type && main_channel.get()) {
            main_channel->setChannelTitle(port&0xFFFF, strings);
            return 1;
        }

        return 0;
    }

    int D3DRenderer::setChannelStrings(int port, XPR_MCVR_StringType type, char **strings, int count)
    {
        if(XPR_MCVR_STRING_TYPE_HINT == type && nullptr!=strings && 4==count) {
            memcpy_s(_camera_state_string->no_device_string, 24, strings[0], strlen(strings[0])+1);
            memcpy_s(_camera_state_string->buffering_string, 24, strings[1], strlen(strings[1])+1);
            memcpy_s(_camera_state_string->interrupt_string, 24, strings[2], strlen(strings[2])+1);
            memcpy_s(_camera_state_string->stopped_string, 24, strings[3], strlen(strings[3])+1);

            return 1;
        }

        return 0;
    }

    int D3DRenderer::setChannelEffect(int port, XPR_MCVR_EffectType effect, int value)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            return main_channel->setChannelEffect(port&0xFFFF, effect, value);
        }

        _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        return 0;
    }

    int D3DRenderer::setChannelEffectF(int port, XPR_MCVR_EffectType effect, float value)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            return main_channel->setChannelEffectF(port&0xFFFF, effect, value);
        }

        _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        return 0;
    }

    int D3DRenderer::getChannelEffect(int port, XPR_MCVR_EffectType effect)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            main_channel->getChannelEffect(port&0xFFFF, effect);
        }

        _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        return 0;
    }

    float D3DRenderer::getChannelEffectF(int port, XPR_MCVR_EffectType effect)
    {
        std::shared_ptr<D3DMainChannel> main_channel = getMainChannel(port>>16);
        if(main_channel.get()) {
            main_channel->getChannelEffectF(port&0xFFFF, effect);
        }

        _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
        return 0.0f;
    }

    bool D3DRenderer::addEventCallback(XPR_MCVR_EventCallback callback, void *user)
    {
        std::pair<XPR_MCVR_EventCallback, void*> new_item;
        new_item.first = callback;
        new_item.second = user;
        AutoLock auto_lock(&_callback_lock);
        _callback_set.insert(new_item);

        return true;
    }

    bool D3DRenderer::delEventCallback(XPR_MCVR_EventCallback callback, void *user)
    {
        std::pair<XPR_MCVR_EventCallback, void*> new_item;
        new_item.first = callback;
        new_item.second = user;
        AutoLock auto_lock(&_callback_lock);
        _callback_set.erase(new_item);

        return true;
    }

    bool D3DRenderer::attachEvent(XPR_MCVR_EventType ev)
    {
        AutoLock auto_lock(&_event_lock);
        _event_set.insert(ev);
        return true;
    }

    bool D3DRenderer::attachAllEvents(void)
    {
        attachEvent(XPR_MCVR_LEFT_CLICK);
        attachEvent(XPR_MCVR_RIGHT_CLICK);
        attachEvent(XPR_MCVR_LEFT_DBCLICK);
        return true;
    }

    bool D3DRenderer::detachEvent(XPR_MCVR_EventType ev)
    {
        AutoLock auto_lock(&_event_lock);
        _event_set.erase(ev);
        return true;
    }

    bool D3DRenderer::detachAllEvents(void)
    {
        AutoLock auto_lock(&_event_lock);
        _event_set.clear();

        return true;
    }

    void D3DRenderer::genericEvent(XPR_MCVR_EventType event, int port, void *data)
    {
        if(_event_set.find(event) != _event_set.end()) {
            std::set<std::pair<XPR_MCVR_EventCallback, void*>>::iterator set_it = _callback_set.begin();
            for(set_it=_callback_set.begin(); set_it!=_callback_set.end(); ++set_it) {
                if(set_it->first) {
                    set_it->first(event, set_it->second, port, data);
                }
            }
        }
    }

    void D3DRenderer::optimizePerformance(bool is_improving)
    {
        std::shared_ptr<D3DMainChannel> main_channel;
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        for(map_it=_channels_map.begin(); map_it!=_channels_map.end(); ++map_it) {
            main_channel = map_it->second;
            if(!main_channel.get()) {
                main_channel.reset();
                continue;
            }
            if(is_improving) {
                if(main_channel->setHighPerfromance()) {
                    _buffer_count = kMinBufferCount;
                }
            } else {
                if(main_channel->setLowConsumption()) {
                    _buffer_count = kMinBufferCount;
                }
            }

            main_channel.reset();
        }
        _optimize_code = OPTIMIZE_OPERATION_NONE;
    }

    bool D3DRenderer::isMainChanValid(int port)
    {
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>>::iterator map_it;
        map_it = _channels_map.find(port);
        return map_it!=_channels_map.end() ? true : false;
    }

    bool D3DRenderer::isSubChanValid(int port)
    {
        std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(port);
        return sub_channel.get() ? true : false;
    }

    bool D3DRenderer::isFallingSleep(void)
    {
        DWORD cur_time = timeGetTime();
        if(cur_time>_previous_time && kSleepThreshold>(cur_time-_previous_time)) {
            return true;
        }
        return false;
    }

    // D3DRenderer
    //=============================================================================
    D3DEffect::D3DEffect(IDirect3DDevice9 *device)
        :_device(device)
        ,_effect(nullptr)
        ,_planar_tech(nullptr)
        ,_uyvy_tech(nullptr)
        ,_x_offset_handle(nullptr)
        ,_y_offset_handle(nullptr)
        ,_sharp_handle(nullptr)
        ,_bright_handle(nullptr)
        ,_defog_handle(nullptr)
        ,_hist_handle(nullptr)
        ,_tex_width(0)
        ,_tex_height(0)
        ,_wnd_ratio(1.0f)
        ,_dest_ratio(1.0f)
        ,_alarm_factor(0.0f)
        ,_is_alarming(false)
    {
        if(_device) {
            initEffect();
        }
    }

    D3DEffect::~D3DEffect()
    {
        SafeRelease(&_effect);
    }

    bool D3DEffect::initEffect(void)
    {
        if(!_device) {
            return false;
        }

        //Get shader version.
        HRESULT hr = S_OK;
        D3DCAPS9 caps;
        hr = _device->GetDeviceCaps(&caps);
        if(FAILED(hr)) {
            return false;
        }
        //Load effect resource.
        void *data = nullptr;
        DWORD size = 0;
        HMODULE hModule = NULL;
        BOOL ret = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
            reinterpret_cast<LPCSTR>(&loopThreadCallback), &hModule);
        if(!ret) {
            return false;
        }
        HRSRC res = FindResource(hModule, MAKEINTRESOURCE(IDR_RCDATA_EFFECT_2), RT_RCDATA);
        if(NULL == res) {
            printf("BBBBBBBBBBBBBBBBBBBBBBBB\n");
            FreeLibrary(hModule);
            return false;
        }
        HGLOBAL hGlobal = LoadResource(hModule, res);
        if(NULL == hGlobal) {
            FreeLibrary(hModule);
            return false;
        }
        size = ::SizeofResource(hModule, res);
        data = ::LockResource(hGlobal);
        if(0==size || NULL==data) {
            FreeLibrary(hModule);
            return false;
        }

        //Create effect.
        CComPtr<ID3DXBuffer> error_buffer;
        hr = D3DXCreateEffect(
            _device, data, size,
            0, 0, D3DXSHADER_DEBUG | D3DXSHADER_OPTIMIZATION_LEVEL3, 0,
            &_effect, &error_buffer);
        FreeLibrary(hModule);
        if(error_buffer) {
            //PRINT_DEBUG_MSG(error_buffer->GetBufferPointer());
        }
        if(FAILED(hr)) {
            return false;
        }

            printf("BBBBBBBBBBBBBBBBBBBBBBBB XXXXXXXXXXX\n");

        //Get handle.
        _x_offset_handle = _effect->GetParameterByName(NULL, "OffsetX");
        _y_offset_handle = _effect->GetParameterByName(NULL, "OffsetY");
        _wnd_ratio_handle = _effect->GetParameterByName(NULL, "WindowRatio");
        _dest_ratio_handle = _effect->GetParameterByName(NULL, "DestRatio");
        _alarm_handle = _effect->GetParameterByName(NULL, "AlarmOffset");
        _sharp_handle = _effect->GetParameterByName(NULL, "SharpFactor");
        _bright_handle = _effect->GetParameterByName(NULL, "BrightFactor");
        _defog_handle = _effect->GetParameterByName(NULL, "DefogFactor");
        _hist_handle = _effect->GetParameterByName(NULL, "HistFactor");
        _planar_tech = _effect->GetTechniqueByName("Tech_Planar");
        _uyvy_tech = _effect->GetTechniqueByName("Tech_UYVY");

        hr = _effect->SetTechnique(_planar_tech);
        _effect->SetFloat(_alarm_handle, _alarm_factor);
        _effect->SetFloat(_wnd_ratio_handle, _wnd_ratio);
        _effect->SetFloat(_dest_ratio_handle, _dest_ratio);
        return true;
    }

    void D3DEffect::onLostDevice(void)
    {
        if(_effect) {
            _effect->OnLostDevice();
        }
    }

    void D3DEffect::onResetDevice()
    {
        if(_effect) {
            _effect->OnResetDevice();
        }
    }

    bool D3DEffect::setOffset(const int width, const int height)
    {   
        if(width==_tex_width && height==_tex_height) {
            return true;
        }
        _tex_width = width;
        _tex_height = height;
        if(!_effect) {
            return false;
        }

        float offset_x = 1.0f / static_cast<float>(_tex_width);
        float offset_y = 1.0f / static_cast<float>(_tex_height);
        _effect->SetFloat(_x_offset_handle, offset_x);
        _effect->SetFloat(_y_offset_handle, offset_y);
        return true;
    }

    bool D3DEffect::setRatio(const float wnd_ratio, const float dest_ratio)
    {
        if(wnd_ratio==_wnd_ratio && dest_ratio==_dest_ratio) {
            return true;
        }
        _wnd_ratio = wnd_ratio;
        _dest_ratio = dest_ratio;
        if(!_effect) {
            return false;
        }
        _effect->SetFloat(_wnd_ratio_handle, _wnd_ratio);
        _effect->SetFloat(_dest_ratio_handle, _dest_ratio);

        return true;
    }

    bool D3DEffect::convertColor(XPR_AVPixelFormat format, bool is_alarming)
    {
        if(!_effect || XPR_AV_PIX_FMT_YUV420P!=format) {
            return false;
        }

        HRESULT hr = S_OK;
        if(is_alarming != _is_alarming) {
            _is_alarming = is_alarming;
            _alarm_factor = kAlarmOffset - _alarm_factor;
            hr = _effect->SetFloat(_alarm_handle, _alarm_factor);
        }
        hr = _effect->Begin(nullptr, 0);
        hr = _effect->BeginPass(0);
        hr = _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        hr = _effect->EndPass();
        hr = _effect->End();
        
        return true;
    }

    bool D3DEffect::drawRGB(void)
    {
        if(!_effect) {
            return false;
        }

        HRESULT hr = S_OK;
        hr = _effect->Begin(nullptr, 0);
        hr = _effect->BeginPass(1);
        hr = _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        hr = _effect->EndPass();
        hr = _effect->End();

        return true;
    }

    bool D3DEffect::sharpenImage(int val)
    {
        if((val-0)*(100-val) < 0) {
            return false;
        }

        float factor = static_cast<float>(val) / 100.0f;
        _effect->SetFloat(_sharp_handle, factor);
        _effect->Begin(nullptr, 0);
        _effect->BeginPass(2);
        _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        _effect->EndPass();
        _effect->End();
        return true;
    }

    bool D3DEffect::brightenImage(int val)
    {
        if((val-0)*(100-val) < 0) {
            return false;
        }
        float factor = static_cast<float>(val) / 100.0f;
        _effect->SetFloat(_bright_handle, factor);
        _effect->Begin(nullptr, 0);
        _effect->BeginPass(3);
        _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        _effect->EndPass();
        _effect->End();
        return true;
    }

    bool D3DEffect::stretchHist(float *range)
    {
        if(!range || !_effect) {
            return false;
        }
        
        _effect->SetFloatArray(_hist_handle, range, 2);
        _effect->Begin(nullptr, 0);
        _effect->BeginPass(4);
        _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        _effect->EndPass();
        _effect->End();

        return true;
    }

    bool D3DEffect::equalHist()
    {
        if(!_effect) {
            return false;
        }

        _effect->Begin(nullptr, 0);
        _effect->BeginPass(5);
        _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        _effect->EndPass();
        _effect->End();

        return true;
    }

} // namespace XD