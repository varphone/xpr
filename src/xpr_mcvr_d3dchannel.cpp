#include <d3dx9.h>
#include <MMSystem.h>
#include <atlbase.h>
#include <Commctrl.h>   //窗口子类化
#include <xpr/xpr_mcvr.h>
#include "xpr_mcvr_memcpy.h"
#include "xpr_mcvr_d3dchannel.hpp"
#include "xpr_mcvr_channelattribute.hpp"
#include "xpr_mcvr_linepainter.hpp"
#include "xpr_mcvr_safefree.hpp"
#include "xpr_mcvr_singleevent.hpp"
#include "xpr_mcvr_semaphore.hpp"

extern int _last_mcvr_error;
extern int _optimize_code;

//子通道操作级别
const int kSubChanOptimizing =  0x00FF;
const int kSubChanCreation =    0x0FFF;
const int kSubChanReset =       0xFFFF;
//纹理缓冲池阈值
const int kTexPoolOverload = 512 * 1024 * 1024; // 512MB
const int kTexPoolHigh = 256 * 1024 * 1024; // 256MB
const int kTexPoolLow = 64 * 1024 * 1024;  // 64MB
//纹理缓冲池大小
static int _texture_pool_size = 0; 
//纹理顶点
#define D3DFVF_VERTEX_TEXTURE (D3DFVF_XYZ | D3DFVF_TEX1)
struct CUSTOMVERTEX_TEXTURE{
    D3DXVECTOR3 pos;
    D3DXVECTOR2 coord[1];
};
//最大连续丢帧数目
const int kMaxDropFrames = 1;

// D3DSrcTexture
//=============================================================================
struct D3DSrcTexture
{
    IDirect3DTexture9 *y_texture;
    IDirect3DTexture9 *u_texture;
    IDirect3DTexture9 *v_texture;
    D3DLOCKED_RECT *y_rect;
    D3DLOCKED_RECT *u_rect;
    D3DLOCKED_RECT *v_rect;
    DWORD device_time;  //ms

    D3DSrcTexture(void)
        :y_texture(nullptr)
        ,u_texture(nullptr)
        ,v_texture(nullptr)
        ,y_rect(nullptr)
        ,u_rect(nullptr)
        ,v_rect(nullptr)
        ,device_time(0)
    {
    }

    ~D3DSrcTexture()
    {
        if(y_texture) {
            y_texture->UnlockRect(0);
            y_texture->Release();
            y_texture = nullptr;
        }
        if(u_texture) {
            u_texture->UnlockRect(0);
            u_texture->Release();
            u_texture = nullptr;
        }
        if(v_texture) {
            v_texture->UnlockRect(0);
            v_texture->Release();
            v_texture = nullptr;
        }

        if(y_rect) {
            delete y_rect;
            y_rect = nullptr;
        }
        if(u_rect) {
            delete u_rect;
            u_rect = nullptr;
        }
        if(v_rect) {
            delete v_rect;
            v_rect = nullptr;
        }

        device_time = 0;
    }
};

namespace XD
{
    // D3DSubChannel
    //=============================================================================
    D3DSubChannel::D3DSubChannel(D3DMainChannel *main_hannnel, int port, IDirect3DDevice9 *device, int bufs)
        :_operation_level(0)
        ,_operation_lock()
        ,_main_channnel(main_hannnel)
        ,_channel_port(port)
        ,_device(device)
        ,_buffer_count(bufs)
        ,_front_lock()
        ,_back_lock()
        ,_frame_width(0)
        ,_frame_height(0)
        ,_aspect_ratio(1.0f)
        ,_pixel_format(XPR_AV_PIX_FMT_YUV420P)
        ,_hist_count(0)
        ,_pixel_table(nullptr)
        ,_renderer_rate(1.0f)
        ,_device_time(0)
        ,_local_time(0)
        ,_base_local_time(0)
        ,_base_device_time(0)
        ,_max_drop_frames(0)
        ,_drop_times(0)
        ,_snapshot_path(nullptr)
        ,_is_created(false)
        ,_is_shotting(false)
    {
        _hist_factor[0] = 0.0f;
        _hist_factor[1] = 0.0f;
    }

    D3DSubChannel::~D3DSubChannel()
    {
        resetChannel();
    }

    void D3DSubChannel::setCameraResolution(const int width, const int height)
    {
        _frame_width = width;
        _frame_height = height;
        if(0 < _frame_width*_frame_height) {
            _aspect_ratio = static_cast<float>(_frame_width) / static_cast<float>(_frame_height);
        } else {
            _aspect_ratio = 1.0f;
        }
    }

    bool D3DSubChannel::inputData(XPR_AVFrame *av_frame)
    {
        //暂只支持YV12
        if(!av_frame || !av_frame->datas[0]
        || 0==av_frame->width*av_frame->height
            || XPR_AV_PIX_FMT_YUV420P!=av_frame->format) {
                _last_mcvr_error = XPR_MCVR_ERR_INVALID_INPUT_DATA;
                return false;
        }

        av_frame->pitches[0] = 0!=av_frame->pitches[0] ? av_frame->pitches[0] : av_frame->width;
        av_frame->pitches[1] = 0!=av_frame->pitches[1] ? av_frame->pitches[1] : av_frame->width/2;
        av_frame->pitches[2] = 0!=av_frame->pitches[2] ? av_frame->pitches[2] : av_frame->width/2;

        bool ret = true;
        if(av_frame->width==_frame_width
            &&av_frame->height==_frame_height
            /*&&_pixel_format==av_frame->format*/) {
                return fillData(av_frame);
        } else {
            if(_is_created) {
                resetChannel();
            }

            setCameraResolution(av_frame->width, av_frame->height);
            ret = createTextures();
            return fillData(av_frame);
        }
    }

    void D3DSubChannel::resetChannel(void)
    {
        if(!bindOperation(kSubChanReset)) {
            return;
        }
        resetSnapshotFlag();
        //循环，直到操作级别等于重置级别，可保证没有其他操作正在进行
        while(kSubChanReset < _operation_level) {
            Sleep(10);
        }
        
        //取出前置队列和后备队列所有项
        int count = 0;
        D3DSrcTexture *src_array[2*kMaxBufferCount];
        memset(src_array, 0, 2*kMaxBufferCount*sizeof(D3DSrcTexture*));
        _back_lock.Lock();
        while(!_back_tex_que.empty()) {
            src_array[count] = _back_tex_que.front();
            ++count;
            _back_tex_que.pop();
        }
        _back_lock.Unlock();
        _front_lock.Lock();
        while(!_front_tex_que.empty()) {
             src_array[count] = _front_tex_que.front();
             _front_tex_que.pop_front();
            ++count;
        }
        _front_lock.Unlock();    
        releaseOperation(kSubChanReset);
        
        //清空纹理向量
        for(int i=0; i<count; ++i) {
            SafeDelete(&src_array[i]);
        }
        //YUV420P
        _texture_pool_size -= count * _frame_width * _frame_height * 3 / 2;
        SafeRelease(&_pixel_table);
        if(kMinBufferCount==_buffer_count && kTexPoolLow>=_texture_pool_size) {
            _optimize_code = OPTIMIZE_OPERATION_IMPROVING;
        }

        setCameraResolution(0, 0);
        _is_created = false;
    }

    bool D3DSubChannel::createTextures(void)
    {
        if(_is_created || kTexPoolOverload<=_texture_pool_size) {
            _last_mcvr_error = XPR_MCVR_ERR_TEXTURE_CREATE_FAILED;
            return false;
        }

        if(!bindOperation(kSubChanCreation)) {
            return false;
        }

        switch(_pixel_format) {
        case XPR_AV_PIX_FMT_YUV420P:
            {
                AutoLock auto_lock(&_back_lock);
                for(int i=0; i<_buffer_count; ++i) {
                    D3DSrcTexture *src_texture = createYUV420PTexture();
                    if(src_texture) {
                        _back_tex_que.push(src_texture);
                        _texture_pool_size += _frame_width * _frame_height * 3 / 2;
                        _is_created = true;
                    }
                }
            }
            break;
        default:
            break;
        }

        releaseOperation(kSubChanCreation);
        if(kMaxBufferCount==_buffer_count && kTexPoolHigh<=_texture_pool_size) {
            _optimize_code = OPTIMIZE_OPERATION_REDUCING;
        }
        return _is_created;
    }
    
    D3DSrcTexture *D3DSubChannel::createYUV420PTexture(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return nullptr;
        }

        D3DSrcTexture *src_texture = new D3DSrcTexture();
        if(src_texture) {
            HRESULT hr = S_OK;
            //Y
            hr = _device->CreateTexture(
                _frame_width, _frame_height, kTextureLevel,
                D3DUSAGE_DYNAMIC, D3DFMT_L8,
                D3DPOOL_SYSTEMMEM, &src_texture->y_texture, nullptr);
            if(FAILED(hr)) {
                SafeDelete(&src_texture);
                return nullptr;
            }
            //U
            hr = _device->CreateTexture(
                _frame_width/2, _frame_height/2, kTextureLevel,
                D3DUSAGE_DYNAMIC, D3DFMT_L8,
                D3DPOOL_SYSTEMMEM, &src_texture->u_texture, nullptr);
            if(FAILED(hr)) {
                SafeDelete(&src_texture);
                return nullptr;
            }
            //V
            hr = _device->CreateTexture(
                _frame_width/2, _frame_height/2, kTextureLevel,
                D3DUSAGE_DYNAMIC, D3DFMT_L8,
                D3DPOOL_SYSTEMMEM, &src_texture->v_texture, nullptr);
            if(FAILED(hr)) {
                SafeDelete(&src_texture);
                return nullptr;
            }
            //Lock textures.
            src_texture->y_rect = new D3DLOCKED_RECT();
            src_texture->u_rect = new D3DLOCKED_RECT();
            src_texture->v_rect = new D3DLOCKED_RECT();
            src_texture->y_texture->LockRect(0, src_texture->y_rect, nullptr, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
            src_texture->u_texture->LockRect(0, src_texture->u_rect, nullptr, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
            src_texture->v_texture->LockRect(0, src_texture->v_rect, nullptr, D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);

            return src_texture;
        }

        return nullptr;
    }

    bool D3DSubChannel::fillData(XPR_AVFrame *av_frame)
    {
        //转移操作需单独判断操作级别
        {
            AutoLock auto_lock(&_operation_lock);
            if(_buffer_count <= (_operation_level+1)) {
                return false;
            }
            ++_operation_level;
        }

        bool ret = false;
        D3DSrcTexture *src_texture = nullptr;
        _back_lock.Lock();
        if(!_back_tex_que.empty()) {
            src_texture = _back_tex_que.front();
            _back_tex_que.pop();
            _back_lock.Unlock();
        }
        else {
            _back_lock.Unlock();
            AutoLock auto_lock(&_front_lock);
            if(!_front_tex_que.empty()) {
                src_texture = _front_tex_que.front();
                _front_tex_que.pop_front();
                if(_main_channnel) {
                    _main_channnel->waitData(0);
                }
                //TODO: 丢帧策略
                ++_max_drop_frames;
            } else {
                releaseOperation(1);
                _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
                return false;
            }
        }

        ret = fillYUV420PTexture(src_texture, av_frame);
        if(ret) {
            src_texture->device_time = static_cast<DWORD>(av_frame->pts/1000); //microsecond to millisecond.
            //pushToFront(src_texture);
            _front_lock.Lock();
            _front_tex_que.push_back(src_texture);
            _front_lock.Unlock();

            if(_main_channnel) {
                _main_channnel->postData();
            }
        } else {
            pushToBack(src_texture);
        }

        releaseOperation(1);
        return ret;
    }

    bool D3DSubChannel::fillYUV420PTexture(D3DSrcTexture *src_texture, XPR_AVFrame *av_frame)
    {
        if(!av_frame || !src_texture
            ||!src_texture->y_texture
            ||!src_texture->u_texture
            ||!src_texture->v_texture) {
                _last_mcvr_error = XPR_MCVR_ERR_NULL_TEXTURE;
                return false;
        }

        BYTE *src_data = nullptr;
        BYTE *dest_data = nullptr;
        int src_pitch = 0;
        HRESULT hr = S_OK;

        //Y
        src_data = reinterpret_cast<BYTE*>(av_frame->datas[0]);
        dest_data = reinterpret_cast<BYTE*>(src_texture->y_rect->pBits);
        src_pitch = src_texture->y_rect->Pitch;
        if(av_frame->pitches[0]==_frame_width && src_pitch==_frame_width) {
            unaligned_memcpy_sse2(dest_data, src_data, _frame_width*_frame_height);
        } else {
            for(int i=0; i<_frame_height; ++i) {
                unaligned_memcpy_sse2(dest_data+i*src_pitch, src_data+i*av_frame->pitches[0], _frame_width);
            }
        }

        //U
        src_data = reinterpret_cast<BYTE*>(av_frame->datas[1]);
        dest_data = reinterpret_cast<BYTE*>(src_texture->u_rect->pBits);
        src_pitch = src_texture->u_rect->Pitch;
        if(av_frame->pitches[1]==_frame_width/2 && src_pitch==av_frame->pitches[1]) {
            unaligned_memcpy_sse2(dest_data, src_data, _frame_width*_frame_height/4);
        } else {
            for(int i=0; i<_frame_height/2; ++i) {
                unaligned_memcpy_sse2(dest_data+i*src_pitch, src_data+i*av_frame->pitches[1], _frame_width/2);
            }
        }

        //V
        src_data = reinterpret_cast<BYTE*>(av_frame->datas[2]);
        dest_data = reinterpret_cast<BYTE*>(src_texture->v_rect->pBits);
        src_pitch = src_texture->v_rect->Pitch;
        if(av_frame->pitches[2]==_frame_width/2 && src_pitch==av_frame->pitches[2]) {
            unaligned_memcpy_sse2(dest_data, src_data, _frame_width*_frame_height/4);
        } else {
            for(int i=0; i<_frame_height/2; ++i) {
                unaligned_memcpy_sse2(dest_data+i*src_pitch, src_data+i*av_frame->pitches[2], _frame_width/2);
            }
        }

        return true;
    }

    UINT D3DSubChannel::getParams()
    {
        //The first 16 bits for width.
        //The second 12 bits for height.
        //The last 4 bits for format bytes.
        UINT size = 0;
        size = (static_cast<DWORD>(0xFFFF&_frame_width)) << 16;
        size += (((static_cast<DWORD>(_frame_height))<<4) & 0xFFF0) + 0x1;
        return size;
    }

    void D3DSubChannel::setRate(float rate)
    {
        _renderer_rate = rate;
    }

    void D3DSubChannel::resetPTS(void)
    {
        _device_time = 0;
        _local_time = 0;
    }

    bool D3DSubChannel::updateDestTexture(D3DDestTexture *dest_texture)
    {
        if(!dest_texture || !_device) {
            _last_mcvr_error = XPR_MCVR_ERR_UNKNOWN;
            return false;
        }
        
        //转移操作需单独判断操作级别
        {
            AutoLock auto_lock(&_operation_lock);
            if(_buffer_count <= _operation_level) {
                return false;
            }
            ++_operation_level;
        }

        D3DSrcTexture *src_texture = nullptr;
        _front_lock.Lock();
        if(!_front_tex_que.empty()) {
            src_texture = popFromFront();
        } else {
            _front_lock.Unlock();
            releaseOperation(1);
            return false;
        }

        bool ret = true;
        FRAME_TRANSFER_ACTION frame_action = getFrameAction(src_texture);
        switch(frame_action) {
        case FRAME_TRANSFER_DISPLAY:
            break;
        case FRAME_TRANSFER_DROP:
            {
                pushToBack(src_texture);
                releaseOperation(1);
            }
            return false;
        case FRAME_TRANSFER_HOLD:
            {
                pushToFront(src_texture);
                releaseOperation(1);
            }
            return false;
        default:
            releaseOperation(1);
            return false;
        }

        HRESULT hr = S_OK;
        //YV12
        if(src_texture) {
            if(!src_texture->y_texture||!src_texture->u_texture||!src_texture->v_texture
                ||!dest_texture->y_texture||!dest_texture->u_texture||!dest_texture->v_texture) {
                pushToBack(src_texture);
                releaseOperation(1);
                return false;
            }
            
            hr = _device->UpdateTexture(src_texture->y_texture, dest_texture->y_texture);
            hr = _device->SetTexture(0, dest_texture->y_texture);
            if(FAILED(hr)) {
                //WPRINT_DEBUG_MSG(L"Failed to set texture.");
            }
            src_texture->y_texture->AddDirtyRect(NULL);
            
            if(SUCCEEDED(hr)) {
                hr = _device->UpdateTexture(src_texture->u_texture, dest_texture->u_texture);
                hr = _device->SetTexture(1, dest_texture->u_texture);
                src_texture->u_texture->AddDirtyRect(NULL);
            }
            if(SUCCEEDED(hr)) {
                hr = _device->UpdateTexture(src_texture->v_texture, dest_texture->v_texture);
                hr = _device->SetTexture(2, dest_texture->v_texture);
                src_texture->v_texture->AddDirtyRect(NULL);
            }
            ret = SUCCEEDED(hr) ? true : false;
        } else {
            ret = false;
        }
        //根据直方图计算拉伸参数
        if(ret) {
           defogImage(src_texture); 
        }

        pushToBack(src_texture);
        releaseOperation(1);
        return ret;
    }

    void D3DSubChannel::takeSnapshot(const char *path)
    {
        if(path) {
            SafeDeleteArray(&_snapshot_path);
            _snapshot_path = new char[strlen(path)+1];
            if(_snapshot_path) {
                memcpy_s(_snapshot_path, strlen(path)+1, path, strlen(path)+1);
                _is_shotting = true;
            }
        }
    }

    void D3DSubChannel::resetSnapshotFlag()
    {
        _is_shotting = false;
        SafeDeleteArray(&_snapshot_path);
    }

    void D3DSubChannel::defogImage(D3DSrcTexture *src_texture)
    {
        if(!_main_channnel || 0>=_main_channnel->getDefogFactor(_channel_port) || !src_texture) {
            return;
        }

        switch (_main_channnel->getDefogMode(_channel_port))
        {
        case DEFOG_STRETCH_HIST:
            stretchHist(src_texture, _main_channnel->getDefogFactor(_channel_port));
            break;
        case DEFOG_EQUAL_HIST:
            equalHist(src_texture);
            break;
        default:
            break;
        }
    }

    bool D3DSubChannel::stretchHist(D3DSrcTexture *src_texture, int value)
    {
        if(!src_texture || !src_texture->y_rect || !src_texture->y_rect->pBits) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_TEXTURE;
            return false;
        }
        float factor = static_cast<float>(value) / 100.0f;

        int hist[256];
        memset(hist, 0, 256 * sizeof(int));
        BYTE *data = reinterpret_cast<BYTE*>(src_texture->y_rect->pBits);
        int pitch = src_texture->y_rect->Pitch;
        BYTE cur_pixel;
        memset(hist, 0, 256*sizeof(int));
        for(int i=0; i<_frame_height; ++i) {
            for(int j=0; j<_frame_width; ++j) {
                cur_pixel = *(data+i*pitch+j);
                ++hist[cur_pixel];
            }
        }
        for(int i=1; i<256; ++i) {
            hist[i] += hist[i-1];
        }

        double sum = static_cast<double>(_frame_width) 
                   * static_cast<double>(_frame_height);
        //以0.01%像素为强度单位
        int threshold = static_cast<int>(static_cast<double>(factor)*sum*0.01);
        BYTE max_pixel, min_pixel;
        for(int i=0; i<255; ++i) {
            if(threshold < hist[i]) {
                min_pixel = static_cast<BYTE>(i);
                break;
            }
        }
        for(int i=255; i>0;--i) {
            if(threshold < (_frame_width*_frame_height-hist[i])) {
                max_pixel = static_cast<BYTE>(i);
                break;
            }
        }
        _hist_factor[0] = static_cast<float>(min_pixel) / 256.0f;
        _hist_factor[1] = static_cast<float>(max_pixel) / 256.0f;

        return true;
    }

    bool D3DSubChannel::equalHist(D3DSrcTexture *src_texture)
    {
        if (!_device || !src_texture || !src_texture->y_rect || !src_texture->y_rect->pBits) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_TEXTURE;
            return false;
        }
        
        int hist[256];
        memset(hist, 0, 256 * sizeof(int));
        BYTE *data = reinterpret_cast<BYTE*>(src_texture->y_rect->pBits);
        int pitch = src_texture->y_rect->Pitch;
        BYTE cur_pixel;
        BYTE max_pixel=0, min_pixel=255;
        memset(hist, 0, 256 * sizeof(int));
        for (int i = 0; i < _frame_height; ++i) {
            for (int j = 0; j < _frame_width; ++j) {
                cur_pixel = *(data + i*pitch + j);
                ++hist[cur_pixel];
                max_pixel = max(cur_pixel, max_pixel);
                min_pixel = min(cur_pixel, min_pixel);
            }
        }
        
        HRESULT hr = S_OK;
        if (!_pixel_table) {
            hr = _device->CreateTexture(
                256, 1, kTextureLevel, D3DUSAGE_DYNAMIC, D3DFMT_L8,
                D3DPOOL_SYSTEMMEM, &_pixel_table, NULL);
            if(FAILED(hr)) {
                return false;
            }
        }

        D3DLOCKED_RECT rect;
        hr = _pixel_table->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
        if (FAILED(hr)) {
            return false;
        }
        data = reinterpret_cast<BYTE*>(rect.pBits);
        pitch = rect.Pitch;

        //TODO:
        max_pixel = 225;
        max_pixel -= min_pixel;
        float scale = 0.0f;
        double sum = static_cast<double>(_frame_width) * static_cast<double>(_frame_height);
        scale = static_cast<float>(static_cast<double>(hist[0]) / sum);

        data[0] = static_cast<BYTE>(min(225.0f, scale*static_cast<float>(max_pixel)+static_cast<float>(min_pixel)));
        for (int i = 1; i < 256; ++i) {
            hist[i] += hist[i-1];
            scale = static_cast<float>(static_cast<double>(hist[i]) / sum);
            data[i] = static_cast<BYTE>(min(225.0f, scale*static_cast<float>(max_pixel)+static_cast<float>(min_pixel)));
        }
        _pixel_table->UnlockRect(0);
        return true;
    }

    float *D3DSubChannel::getHistFactor(void)
    {
        return _hist_factor;
    }

    bool D3DSubChannel::updatePixelTable(IDirect3DTexture9 *dest_table)
    {
        if (!_device || !_pixel_table || !dest_table) {
            return false;
        }

        HRESULT hr = S_OK;
        
        hr = _device->UpdateTexture(_pixel_table, dest_table);
        //复用一号纹理舞台
        hr = _device->SetTexture(1, dest_table);
        return SUCCEEDED(hr) ? true : false;
    }

    bool D3DSubChannel::setHighPerfromance(void)
    {
        if(kMaxBufferCount <= _buffer_count) {
            return true;
        }
        if(!bindOperation(kSubChanOptimizing)) {
            return false;
        }
        //循环，一直等到操作级别等于优化级别，保证没有其他操作正在进行
        while(kSubChanOptimizing < _operation_level) {
            Sleep(5);
        }

        const int delta_count = kMaxBufferCount - kMinBufferCount;
        D3DSrcTexture *src_tex[delta_count];
        for(int i=0; i<(delta_count); ++i) {
            src_tex[i] = nullptr;
        }
        for(int i=0; i<(delta_count); ++i) {
            src_tex[i] = createYUV420PTexture();
            if(!src_tex[i]) {
                for(int j=0; j<i; ++i) {
                    SafeDelete(&src_tex[j]);
                }
                releaseOperation(kSubChanOptimizing);
                return false;
            }
        }

        _back_lock.Lock();
        for(int i=0; i<(delta_count); ++i) {
            _back_tex_que.push(src_tex[i]);
        }
        _buffer_count = kMaxBufferCount;
        _back_lock.Unlock();

        _texture_pool_size += delta_count * _frame_width * _frame_height * 3 / 2;
        releaseOperation(kSubChanOptimizing);
        return true;
    }

    bool D3DSubChannel::setLowConsumption(void)
    {
        if(kMinBufferCount >= _buffer_count) {
            return true;
        }
        if(!bindOperation(kSubChanOptimizing)) {
            return false;
        }
        //循环，一直等到操作级别等于优化级别，保证没有其他操作正在进行
        while(kSubChanOptimizing < _operation_level) {
            Sleep(5);
        }

        const int delta_count = kMaxBufferCount - kMinBufferCount;
        int count = 0;
        int times = 0;  //循环次数，防止意外死锁
        std::vector<D3DSrcTexture*> tex_verctor;
        while(count<delta_count && 10>times) {
            ++times;
            _back_lock.Lock();
            if(!_back_tex_que.empty()) {
                tex_verctor.push_back(_back_tex_que.front());
                _back_tex_que.pop();
                _back_lock.Unlock();
                ++count;
                continue;
            }
            _back_lock.Unlock();

            _front_lock.Lock();
            if(!_front_tex_que.empty()) {
                tex_verctor.push_back(_front_tex_que.front());
                _front_tex_que.pop_front();
                _front_lock.Unlock();
                ++count;
                continue;
            }
            _front_lock.Unlock();
        }
        _buffer_count = kMinBufferCount;
        
        //清空纹理向量
        D3DSrcTexture *cur_tex = nullptr;
        while(!tex_verctor.empty()) {
            cur_tex = tex_verctor.back();
            tex_verctor.pop_back();
            SafeDelete(&cur_tex);
        }
        _texture_pool_size -= count * _frame_width * _frame_height * 3 / 2;

        releaseOperation(kSubChanOptimizing);
        return true;
    }

    FRAME_TRANSFER_ACTION D3DSubChannel::getFrameAction(D3DSrcTexture *src_tex)
    {
        if(!src_tex) {
            return FRAME_TRANSFER_NONE;
        }
        return FRAME_TRANSFER_DISPLAY;

        //时间戳已重置
        if(0 == _device_time*_local_time) {
            _device_time = src_tex->device_time;
            _local_time = timeGetTime();
            return FRAME_TRANSFER_DISPLAY;
        }

        DWORD device_time = src_tex->device_time;
        DWORD local_time = timeGetTime();
        int dur_time = static_cast<int>(
            static_cast<int64_t>(device_time)
            - static_cast<int64_t>(_device_time));
        int delay_time = static_cast<int>(
            static_cast<int64_t>(local_time)
            - static_cast<int64_t>(_local_time));

        //时间戳已重置
        if(0>=dur_time || 0>=delay_time) {
            _local_time = local_time;
            _device_time = device_time;
            return FRAME_TRANSFER_DISPLAY;
        }
        if(0.000001f < _renderer_rate) {
            dur_time = static_cast<int>(static_cast<float>(dur_time)/_renderer_rate);
        }
        delay_time = dur_time - delay_time;

        int sleep_time = 10;
        //显示
        //-dur_time-4 <= delay_time <=4
        //if(0 <= (sleep_time-delay_time)*(dur_time+sleep_time+delay_time)) {
        if(sleep_time >= delay_time) {
            _local_time = timeGetTime();
            _device_time = device_time;
            _drop_times = 0;

            return FRAME_TRANSFER_DISPLAY;
        }
        return FRAME_TRANSFER_HOLD;

        //持有
        //4 < delay_time < dur_time+4
        if(0 < (delay_time-sleep_time)*(dur_time+sleep_time-delay_time)) {
            return FRAME_TRANSFER_HOLD;
        }
        //丢弃
        else {
            _local_time = local_time;
            _device_time = _device_time;

            return FRAME_TRANSFER_DROP;
            ++_drop_times;
            if(2 > _drop_times) {
                return FRAME_TRANSFER_DROP;
            } else {
                _drop_times = 0;
                return FRAME_TRANSFER_DISPLAY;
            }
            
        }

        return FRAME_TRANSFER_NONE;
    }

    bool D3DSubChannel::bindOperation(int level)
    {
        AutoLock auto_lock(&_operation_lock);
        if(_operation_level >= level) {
            return false;
        }
        _operation_level += level;
        return true;
    }

    void D3DSubChannel::releaseOperation(int level)
    {
        AutoLock auto_lock(&_operation_lock);
        _operation_level -= level;
    }

    void D3DSubChannel::pushToBack(D3DSrcTexture *src_texture)
    {
        _back_lock.Lock();
        _back_tex_que.push(src_texture);
        _back_lock.Unlock();
    }

    D3DSrcTexture *D3DSubChannel::popFromBack(void)
    {
        D3DSrcTexture *src_texture = _back_tex_que.front();
        _back_tex_que.pop();
        _back_lock.Unlock();

        return src_texture;
    }

    void D3DSubChannel::pushToFront(D3DSrcTexture *src_texture)
    {
        _front_lock.Lock();
        _front_tex_que.push_back(src_texture);
        _front_lock.Unlock();
    }

    D3DSrcTexture *D3DSubChannel::popFromFront(void)
    {
        D3DSrcTexture *src_texture = _front_tex_que.front();
        _front_tex_que.pop_front();
        _front_lock.Unlock();

        return src_texture;
    }

    // D3DMainChannel
    //=============================================================================
    D3DMainChannel::D3DMainChannel(IDirect3DDevice9 *device, void *hwnd, int rows, int cols, int bufs)
        :_device(device)
        ,_region_vertex(nullptr)
        ,_mission_lock()
        ,_source_port(0x00000000)
        ,_channel_hwnd(reinterpret_cast<HWND>(hwnd))
        ,_rows_count(rows)
        ,_cols_count(cols)
        ,_buffer_count(bufs)
        ,_current_port(0)
        ,_line_painter(nullptr)
        ,_target_texture(nullptr)
        ,_region_ratio(1.0f)
        ,_zoom_ratio(1.0f)
        ,_data_semaphore(nullptr)
    {
        //创建数据信号量
        _data_semaphore = new Semaphore(0);
        //初始化子通道属性表
        for(int i=0; i<kMaxChannelCount+1; ++i) {
            _channel_attributes[i] = nullptr;
        }
        //创建子通道
        std::shared_ptr<D3DSubChannel> sub_chan;
        for(int i=0; i<_rows_count*_cols_count; ++i) {
            sub_chan.reset(new D3DSubChannel(this, i+1, _device, _buffer_count));
            _channels_map.insert(std::pair<int, std::shared_ptr<D3DSubChannel>>(i+1, sub_chan));
            _channel_attributes[i+1] = new ChannelAttribute(XPR_MCVR_CHANNEL_NO_CAMERA, i+1);
        }
        //初始化顶点
        initVertices();
    }

    D3DMainChannel::~D3DMainChannel()
    {
        _channels_map.clear();
        for(int i=0; i<=kMaxChannelCount; ++i) {
            SafeDelete(&_channel_attributes[i]);
        }
        releaseVertices();
        SafeDelete(&_line_painter);
        SafeDelete(&_data_semaphore);
    }

    void D3DMainChannel::createLinePainter(HWND hwnd)
    {
        //创建边框绘制工具对象
        _line_painter = new LinePainter(_device, _channel_hwnd, _rows_count, _cols_count);
    }

    bool D3DMainChannel::initVertices(void)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }

        //Vertices data.
        int count = _rows_count * _cols_count;
        CUSTOMVERTEX_TEXTURE *section_vertices = new CUSTOMVERTEX_TEXTURE[count*4];
        for(int i=0; i<count; ++i) {
            fillSectionVertices(section_vertices, i, _rows_count, _cols_count);
        }
        //Vertex buffer.
        HRESULT hr = S_OK;
        UINT vertex_len = sizeof(CUSTOMVERTEX_TEXTURE) * count * 4;
        hr = _device->CreateVertexBuffer(
            vertex_len, D3DUSAGE_WRITEONLY,
            D3DFVF_VERTEX_TEXTURE, D3DPOOL_DEFAULT,
            &_region_vertex, nullptr);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            SafeDeleteArray(&section_vertices);
            return false;
        }
        void *data = nullptr;
        hr = _region_vertex->Lock(0, 0, &data, D3DLOCK_NOSYSLOCK);
        if(FAILED(hr)) {
            _last_mcvr_error = XPR_MCVR_ERR_VERTEX_CREATE_FAILED;
            SafeDeleteArray(&section_vertices);
            return false;
        }
        memcpy_s(data, vertex_len, section_vertices, vertex_len);
        _region_vertex->Unlock();
        SafeDeleteArray(&section_vertices);
        //FVF
        hr = _device->SetFVF(D3DFVF_VERTEX_TEXTURE);
        //创建主通道渲染目标纹理，并计算各区域宽高比
        createTargetTexture();

        return true;
    }

    bool D3DMainChannel::fillSectionVertices(CUSTOMVERTEX_TEXTURE *vertices, const int index, const int rows, const int cols)
    {
        int count = rows * cols;
        if(!vertices || 0>index || 0>= count) {
            return false;
        }

        vertices[index*4+0].pos.x = static_cast<float>(index%cols) * 16.0f/cols - 8.0f;
        vertices[index*4+0].pos.y = static_cast<float>((count-index-1)/cols) * 9.0f/rows - 4.5f;
        vertices[index*4+0].pos.z = 4.5f;
        vertices[index*4+1].pos.x = static_cast<float>(index%cols) * 16.0f/cols - 8.0f;
        vertices[index*4+1].pos.y = static_cast<float>((count-index-1)/cols+1) * 9.0f/rows - 4.5f;
        vertices[index*4+1].pos.z = 4.5f;
        vertices[index*4+2].pos.x = static_cast<float>(index%cols+1) * 16.0f/cols - 8.0f;
        vertices[index*4+2].pos.y = static_cast<float>((count-index-1)/cols) * 9.0f/rows - 4.5f;
        vertices[index*4+2].pos.z = 4.5f;
        vertices[index*4+3].pos.x = static_cast<float>(index%cols+1) * 16.0f/cols - 8.0f;
        vertices[index*4+3].pos.y = static_cast<float>((count-index-1)/cols+1) * 9.0f/rows - 4.5f;
        vertices[index*4+3].pos.z = 4.5f;
        for(int j=0; j<1; ++j) {
            vertices[index*4+0].coord[j] = D3DXVECTOR2(0.0f, 1.0f);
            vertices[index*4+1].coord[j] = D3DXVECTOR2(0.0f, 0.0f);
            vertices[index*4+2].coord[j] = D3DXVECTOR2(1.0f, 1.0f);
            vertices[index*4+3].coord[j] = D3DXVECTOR2(1.0f, 0.0f);
        }

        return true;
    }

    void D3DMainChannel::createTargetTexture()
    {
        CComPtr<IDirect3DTexture9> target_texture;

        RECT rect;
        GetWindowRect(_channel_hwnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        HRESULT hr = _device->CreateTexture(
            width, height, kTextureLevel, D3DUSAGE_RENDERTARGET,
            D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,
            &target_texture, nullptr);
        if(SUCCEEDED(hr)) {
            SafeRelease(&_target_texture);
            target_texture.CopyTo(&_target_texture);
        }

        //计算通道窗口子区域以及放大区域宽高比
        if(0 < width*height) {
            _zoom_ratio = static_cast<float>(width) / static_cast<float>(height);
        }
        if(0 < _rows_count*_cols_count) {
            _region_ratio = (static_cast<float>(width) * static_cast<float>(_rows_count))
                / (static_cast<float>(height) * static_cast<float>(_cols_count));
        }

        _zoom_ratio = fabs(_zoom_ratio);
        _region_ratio = fabs(_region_ratio);
    }

    void D3DMainChannel::releaseVertices()
    {
        SafeRelease(&_region_vertex);
        SafeRelease(&_target_texture);
    }

    bool D3DMainChannel::inputData(int sub_port, XPR_AVFrame *av_frame)
    {
        std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(sub_port);
        if(!sub_channel.get()) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_CHANNEL;
            return false;
        }

        bool ret = sub_channel->inputData(av_frame);
        if(ret) {
            setChannelState(sub_port, XPR_MCVR_CHANNEL_RENDERING);
        }
        return ret;
    }

    void D3DMainChannel::onLostDevice()
    {
        releaseVertices();
        if(_line_painter) {
            _line_painter->onLostDevice();
        }
    }

    void D3DMainChannel::onResetDevice()
    {
        initVertices();
        if(_line_painter) {
            _line_painter->resetDrawer(_rows_count, _cols_count);
        }
    }

    int D3DMainChannel::clickOnWnd(LPARAM lParam)
    {
        if(0 >= _rows_count*_cols_count) {
            return 0;
        }

        int port = 0;
        if(0 == _source_port) {
            int x = static_cast<int>(lParam&0xFFFF);
            int y = static_cast<int>(lParam>>16) & 0xFFFF;
            RECT rect;
            GetClientRect(_channel_hwnd, &rect);
            int width = (rect.right - rect.left) / _cols_count;
            int height = (rect.bottom - rect.top) / _rows_count;
            port = _cols_count * (y / height) + x / width + 1;
        } else {
            port = _source_port & 0xFFFF;
        }

        return port;
    }

    bool D3DMainChannel::waitData(DWORD ms)
    {
        if(_data_semaphore) {
            return _data_semaphore->wait(ms);
        }

        return false;
    }

    void D3DMainChannel::postData()
    {
        if(_data_semaphore) {
            _data_semaphore->post();
        }
    }

    void D3DMainChannel::postMission(MAIN_CHANNEL_MISSION *mission)
    {
        if(mission) {
            AutoLock auto_lock(&_mission_lock);
            _mission_queue.push(*mission);
        }
    }

    void D3DMainChannel::excuteMissions(void)
    {
        MAIN_CHANNEL_MISSION mission;
        while(true) {
            {
                AutoLock auto_lock(&_mission_lock);
                if(_mission_queue.empty()) {
                    break;
                } else {
                    mission = _mission_queue.front();
                    _mission_queue.pop();
                }
            }

            switch(mission.type) {
            case MAIN_CHANNEL_RESIZE:
                createTargetTexture();
                break;
            case MAIN_CHANNEL_LAYOUT:
                {
                    int rows = mission.data >> 16;
                    int cols = mission.data & 0xFFFF;
                    setChannelLayout(rows, cols);
                }
                break;
            case MAIN_CHANNEl_ZOOM:
                _source_port = mission.data;
                break;
            case  MAIN_CHANNEL_CHOOSE:
                _current_port = mission.data;
                break;
            case MAIN_CHANNEL_BINDING:
                {
                    SafeDelete(&_line_painter);
                    setChannelWindow(reinterpret_cast<HWND>(mission.data));
                    createLinePainter(_channel_hwnd);
                }
                break;
            default:
                break;
            }
        }
    }

    void D3DMainChannel::resetSubChannels(int port)
    {
        if(0 < port) {
            std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(port);
            if(sub_channel.get()) {
                sub_channel->resetChannel();
                setChannelState(port, XPR_MCVR_CHANNEL_NO_CAMERA);
            }
        }
        else if(0 == port)
        {
            for(int i=0; i<_rows_count*_cols_count; ++i) {
                if(_channels_map[i+1].get()) {
                    _channels_map[i+1]->resetChannel();
                    setChannelState(port, XPR_MCVR_CHANNEL_NO_CAMERA);
                }
            }
        }
    }
    
    void D3DMainChannel::setChannelWindow(HWND hwnd)
    {
        if(hwnd==_channel_hwnd || nullptr==hwnd) {
            return;
        }
        RECT pref_rect, cur_rect;
        GetClientRect(hwnd, &cur_rect);
        GetClientRect(_channel_hwnd, &pref_rect);
        _channel_hwnd = hwnd;
        if((pref_rect.right-pref_rect.left)!=(cur_rect.right-cur_rect.left)
            || (pref_rect.bottom-pref_rect.top)!=(cur_rect.bottom-cur_rect.top)) {
                createTargetTexture();
        }
    }

    bool D3DMainChannel::setChannelLayout(int rows, int cols)
    {
        if(0>=rows || 0>=cols || (rows==_rows_count&&cols==_cols_count)) {
            return false;
        }

        std::shared_ptr<D3DSubChannel> sub_chan;
        if(_rows_count*_cols_count < rows*cols) {
            for(int i=(_rows_count*_cols_count+1); i<=(rows*cols); ++i) {
                sub_chan.reset(new D3DSubChannel(this, i, _device, _buffer_count));
                _channels_map.insert(std::pair<int,std::shared_ptr<D3DSubChannel>>(i, sub_chan));
                if(!_channel_attributes[i]) {
                    _channel_attributes[i] = new ChannelAttribute(XPR_MCVR_CHANNEL_NO_CAMERA, i);
                }
            }
        } else {
            for(int i=_rows_count*_cols_count; i>rows*cols; --i) {
                _channels_map.erase(i);
            }
        }

        _rows_count = rows;
        _cols_count = cols;
        onLostDevice();
        onResetDevice();
        return true;
    }

    IDirect3DTexture9 *D3DMainChannel::getTargetTexture()
    {
        if(_is_resizing) {
            createTargetTexture();
            _is_resizing = false;
        }

        return _target_texture;
    }

    HWND D3DMainChannel::getHwnd(void)
    {
        return _channel_hwnd;
    }

    int D3DMainChannel::getChannelsCount(void)
    {
        return _rows_count * _cols_count;
    }

    void D3DMainChannel::setSourcePort(int port)
    {
        _source_port = port;
    }

    std::shared_ptr<D3DSubChannel> D3DMainChannel::getSubChannel(int port)
    {
        std::shared_ptr<D3DSubChannel> sub_channel;
        stdext::hash_map<int, std::shared_ptr<D3DSubChannel>>::iterator map_it;
        map_it = _channels_map.find(port);
        if(map_it != _channels_map.end()) {
            sub_channel = map_it->second;
        }

        return sub_channel;
    }

    bool D3DMainChannel::setRegionSource(int port)
    {
        if(!_device) {
            _last_mcvr_error = XPR_MCVR_ERR_NULL_DEVICE;
            return false;
        }
        _device->SetFVF(D3DFVF_VERTEX_TEXTURE);
        UINT offset = static_cast<UINT>((port-1)*sizeof(CUSTOMVERTEX_TEXTURE)*4);
        HRESULT hr = _device->SetStreamSource(0, _region_vertex, offset, sizeof(CUSTOMVERTEX_TEXTURE));

        return SUCCEEDED(hr) ? true : false;
    }

    bool D3DMainChannel::isAdjusting(int sub_port)
    {
        if(isPortValid(sub_port) && nullptr!=_channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->isAdjusting();
        }

        return false;
    }

    bool D3DMainChannel::isEnhancing(int sub_port)
    {
        if(isPortValid(sub_port) && nullptr!=_channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->isEnhancing();
        }

        return false;
    }

    bool D3DMainChannel::isAlarming(int sub_port)
    {
        if(isPortValid(sub_port) && nullptr!=_channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->isAlarming();
        }

        return false;
    }

    int D3DMainChannel::getDefogFactor(int sub_port)
    {
        if(_channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->getDefogFactor();
        }

        return 0;
    }

    int D3DMainChannel::getDefogMode(int sub_port)
    {
        if(_channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->getDefogMode();
        }

        return DEFOG_STRETCH_HIST;
    }

    RECT D3DMainChannel::getSubRect(int port)
    {
        RECT rect;
        GetClientRect(_channel_hwnd, &rect);
        if(0==_rows_count*_cols_count || port>_rows_count*_cols_count) {
            return rect;
        }
        int right = rect.right;
        int bottom = rect.bottom;

        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        rect.left = ((port-1)%_cols_count) * width / _cols_count;
        rect.right = ((port-1)%_cols_count+1) * width / _cols_count;
        rect.top = ((port-1)/_cols_count) * height / _rows_count;
        rect.bottom = ((port-1)/_cols_count+1) * height / _rows_count;
        if(5 > abs(right-rect.right)) {
            rect.right = right;
        }
        if(5 > abs(bottom-rect.bottom)) {
            rect.bottom = bottom;
        }

        return rect;
    }

    RECT D3DMainChannel::getFullRect()
    {
        RECT rect;
        GetClientRect(_channel_hwnd, &rect);
        return rect;
    }

    RECT D3DMainChannel::getCenterRect()
    {
        RECT rect;
        GetClientRect(_channel_hwnd, &rect);

        float width = static_cast<float>(rect.right - rect.left);
        float height = static_cast<float>(rect.bottom - rect.top);
        float golden_ratio = 0.618f;
        rect.left += static_cast<int>((1.0f-golden_ratio)/2.0f * width);
        rect.right = rect.left + static_cast<int>(width * golden_ratio);
        rect.top += static_cast<int>((1.0f-golden_ratio)/2.0f * height);
        rect.bottom = rect.top + static_cast<int>(height * golden_ratio);

        return rect;
    }

    void D3DMainChannel::drawLines()
    {
        if(!_line_painter) {
            return;
        }
        int main_port = (_source_port>>16) & 0xFFFF;
        int sub_port = _source_port & 0xFFFF;

        if(0==_source_port) {
            _line_painter->drawAllLines(_current_port);
        }
        else if(0<sub_port*(_rows_count*_cols_count+1-sub_port)) {
            _line_painter->drawFullLines(sub_port==_current_port);
        }
    }

    bool D3DMainChannel::setHighPerfromance(void)
    {
        std::shared_ptr<D3DSubChannel> sub_channel;
        stdext::hash_map<int, std::shared_ptr<D3DSubChannel>>::iterator map_it;
        for(map_it=_channels_map.begin(); map_it!=_channels_map.end(); ++map_it) {
            sub_channel = map_it->second;
            if(sub_channel.get()) {
                sub_channel->setHighPerfromance();
            }
            sub_channel.reset();
        }
        _buffer_count = kMaxBufferCount;
        return true;
    }

    bool D3DMainChannel::setLowConsumption(void)
    {
        std::shared_ptr<D3DSubChannel> sub_channel;
        stdext::hash_map<int, std::shared_ptr<D3DSubChannel>>::iterator map_it;
        for(map_it=_channels_map.begin(); map_it!=_channels_map.end(); ++map_it) {
            sub_channel = map_it->second;
            if(sub_channel.get()) {
                sub_channel->setLowConsumption();
            }
            sub_channel.reset();
        }
        _buffer_count = kMinBufferCount;
        
        return true;
    }

    void D3DMainChannel::setZoomPort(int src_port, int dest_port)
    {
        if(isPortValid(src_port) && _channel_attributes[src_port]) {
            _channel_attributes[src_port]->setZoomPort(dest_port);
        }
    }

    int D3DMainChannel::getZoomPort(int sub_port)
    {
        if(isPortValid(sub_port) && _channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->getZoomPort();
        }

        return -1;
    }

    void D3DMainChannel::setChannelState(int sub_port, XPR_MCVR_ChannelState state)
    {
        if(!isPortValid(sub_port)) {
            return;
        }

        if(_channel_attributes[sub_port]) {
            bool res = _channel_attributes[sub_port]->setChannelState(state);
            if(res && XPR_MCVR_CHANNEL_RENDERING!=state){
                std::shared_ptr<D3DSubChannel> sub_channel = getSubChannel(sub_port);
                if(sub_channel.get()) {
                    sub_channel->resetPTS();
                }
            }
        } else {
            _channel_attributes[sub_port] = new ChannelAttribute(state, sub_port);
        }
    }

    XPR_MCVR_ChannelState D3DMainChannel::getChannelState(int sub_port)
    {
        if(isPortValid(sub_port) && _channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->getChannelState();
        }

        return XPR_MCVR_CHANNEL_NO_CAMERA;
    }

    void D3DMainChannel::setChannelTitle(int sub_port, char *title)
    {
        if(!isPortValid(sub_port)) {
            return;
        }

        if(!_channel_attributes[sub_port]) {
            _channel_attributes[sub_port] = new ChannelAttribute(XPR_MCVR_CHANNEL_NO_CAMERA, sub_port);
        }
        if(_channel_attributes[sub_port]) {
            _channel_attributes[sub_port]->setChannelTitle(title);
        }
    }

    char *D3DMainChannel::getChannelTitle(int sub_port)
    {
        if(isPortValid(sub_port) && _channel_attributes[sub_port]) {
            return _channel_attributes[sub_port]->getChannelTitle();
        }

        sprintf_s(_default_title, 8, "<%d>", sub_port);
        return _default_title;
    }

    int D3DMainChannel::setChannelEffect(int sub_port, XPR_MCVR_EffectType effect, int value)
    {
        if(!isPortValid(sub_port) || nullptr==_channel_attributes[sub_port]) {
            return 0;
        }

        switch(effect) {
        case XPR_MCVR_EFFECT_ALL:
            return _channel_attributes[sub_port]->setEffectFlag(value);
        case XPR_MCVR_EFFECT_BRIGHTEN:
            return _channel_attributes[sub_port]->setBrightness(value);
        case XPR_MCVR_EFFECT_SHARPEN:
            return _channel_attributes[sub_port]->setSharpeness(value);
        case XPR_MCVR_EFFECT_HIST_STRETCH:
            _channel_attributes[sub_port]->setDefogMode(DEFOG_STRETCH_HIST);
            return _channel_attributes[sub_port]->setDefogFactor(value);
        case XPR_MCVR_EFFECT_HIST_EQUAL:
            _channel_attributes[sub_port]->setDefogMode(DEFOG_EQUAL_HIST);
            return _channel_attributes[sub_port]->setDefogFactor(value);
        case XPR_MCVR_EFFECT_ENHANCEMENTS:
            return _channel_attributes[sub_port]->setEnhancementFlag(value);
        case XPR_MCVR_EFFECT_ALARM:
            return _channel_attributes[sub_port]->setAlarm(value);
        case XPR_MCVR_EFFECT_SPECIALS:
            return _channel_attributes[sub_port]->setSpecialsFlag(value);
        case XPR_MCVR_EFFECT_LUMINANCE:
            return _channel_attributes[sub_port]->setLuminance(value);
        case XPR_MCVR_EFFECT_CONTRAST:
            return _channel_attributes[sub_port]->setContrast(value);
        case XPR_MCVR_EFFECT_GAMMA:
            return _channel_attributes[sub_port]->setGamma(value);
        case XPR_MCVR_EFFECT_HUE:
            return _channel_attributes[sub_port]->setHue(value);
        case XPR_MCVR_EFFECT_SATURATION:
            return _channel_attributes[sub_port]->setSaturation(value);
        case XPR_MCVR_EFFECT_PROPERTIES:
            return _channel_attributes[sub_port]->setPropertyFlag(value);
        default:
            return 0;
        }
    }

    int D3DMainChannel::setChannelEffectF(int port, XPR_MCVR_EffectType effect, float value)
    {
        return 0;
    }

    int D3DMainChannel::getChannelEffect(int sub_port, XPR_MCVR_EffectType effect)
    {
        if(!isPortValid(sub_port) || nullptr==_channel_attributes[sub_port]) {
            return 0;
        }

        switch(effect) {
        case XPR_MCVR_EFFECT_ALL:
            return _channel_attributes[sub_port]->getEffectFlag();
        case XPR_MCVR_EFFECT_BRIGHTEN:
            return _channel_attributes[sub_port]->getBrightness();
        case XPR_MCVR_EFFECT_SHARPEN:
            return _channel_attributes[sub_port]->getSharpeness();
        case XPR_MCVR_EFFECT_HIST_STRETCH:
            _channel_attributes[sub_port]->setDefogMode(DEFOG_STRETCH_HIST);
            return _channel_attributes[sub_port]->getDefogFactor();
        case XPR_MCVR_EFFECT_HIST_EQUAL:
            _channel_attributes[sub_port]->setDefogMode(DEFOG_EQUAL_HIST);
            return _channel_attributes[sub_port]->getDefogFactor();
        case XPR_MCVR_EFFECT_ENHANCEMENTS:
            return _channel_attributes[sub_port]->getEnhancementFlag();
        case XPR_MCVR_EFFECT_ALARM:
            return _channel_attributes[sub_port]->getAlarm();
        case XPR_MCVR_EFFECT_SPECIALS:
            return _channel_attributes[sub_port]->getSpecialsFlag();
        case XPR_MCVR_EFFECT_LUMINANCE:
            return _channel_attributes[sub_port]->getLuminance();
        case XPR_MCVR_EFFECT_CONTRAST:
            return _channel_attributes[sub_port]->getContrast();
        case XPR_MCVR_EFFECT_GAMMA:
            return _channel_attributes[sub_port]->getGamma();
        case XPR_MCVR_EFFECT_HUE:
            return _channel_attributes[sub_port]->getHue();
        case XPR_MCVR_EFFECT_SATURATION:
            return _channel_attributes[sub_port]->getSaturation();
        case XPR_MCVR_EFFECT_PROPERTIES:
            return _channel_attributes[sub_port]->getPropertyFlag();
        default:
            return 0;
        }
    }

    float D3DMainChannel::getChannelEffectF(int port, XPR_MCVR_EffectType effect)
    {
        return 0.0f;
    }

} // namespace XD