#include <Windows.h>
#include "memcpy.h"
#include "safefree.hpp"
#include "sourcetexture.hpp"

//frame type
#define T_UYVY		1
#define T_YV12		3
#define T_RGB32		7

//Frame Info
struct FRAME_INFO{
    long nWidth;
    long nHeight;
    long nStamp;
    long nType;
    long nFrameRate;
    DWORD dwFrameNum;
};

SourceTexture::SourceTexture(IDirect3DDevice9Ex *device)
    :_device(device)
    ,_front_y_texture(nullptr)
    ,_front_u_texture(nullptr)
    ,_front_v_texture(nullptr)
    ,_back_y_texture(nullptr)
    ,_back_u_texture(nullptr)
    ,_back_v_texture(nullptr)
    ,_frame_width(0)
    ,_frame_height(0)
    ,_color_type(T_YV12)
    ,_is_ready(false)
{
    if(_device) {
        _device->AddRef();
    }
}

SourceTexture::~SourceTexture()
{
    releaseTexture();
    SafeRelease(&_device);
}

SourceTexture *SourceTexture::createSourceTexture(IDirect3DDevice9Ex *device)
{
    if(!device) {
        return nullptr;
    }
    SourceTexture *source_texture = new SourceTexture(device);
    if(source_texture) {
        return source_texture;
    }
    return nullptr;
}

bool SourceTexture::createTexture()
{
    switch(_color_type)
    {
    case T_YV12:
        return createYV12Texture();
    case T_UYVY:
        return createUYVYTexture();
    case T_RGB32:
        return createRGB32Texture();
    default:
        return false;
    }
}

bool SourceTexture::createYV12Texture()
{
    if(!_device) {
        return false;
    }

    HRESULT hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_front_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_back_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    _back_y_texture->LockRect(0, &_y_rect, nullptr, D3DLOCK_NOSYSLOCK);
    
    hr = _device->CreateTexture(
        _frame_width/2, _frame_height/2, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_front_u_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    hr = _device->CreateTexture(
        _frame_width/2, _frame_height/2, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_back_u_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    _back_u_texture->LockRect(0, &_u_rect, nullptr, D3DLOCK_NOSYSLOCK);

    hr = _device->CreateTexture(
        _frame_width/2, _frame_height/2, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_front_v_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    hr = _device->CreateTexture(
        _frame_width/2, _frame_height/2, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_L8,
        D3DPOOL_SYSTEMMEM, &_back_v_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    _back_v_texture->LockRect(0, &_v_rect, nullptr, D3DLOCK_NOSYSLOCK);

    return true;
}

bool SourceTexture::createUYVYTexture()
{
    if(!_device) {
        return false;
    }

    HRESULT hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_A8L8,
        D3DPOOL_SYSTEMMEM, &_front_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_A8L8,
        D3DPOOL_SYSTEMMEM, &_back_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }

    return true;
}

bool SourceTexture::createRGB32Texture()
{
    if(!_device) {
        return false;
    }

    HRESULT hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, &_front_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }
    hr = _device->CreateTexture(
        _frame_width, _frame_height, 0,
        D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, &_back_y_texture, nullptr);
    if(FAILED(hr)) {
        return false;
    }

    return true;
}

void SourceTexture::releaseTexture()
{
    _texture_lock.Lock();
    _is_ready = false;
    SafeRelease(&_front_y_texture);
    SafeRelease(&_front_u_texture);
    SafeRelease(&_front_v_texture);
    clearRect();
    if(_back_y_texture) {
        _back_y_texture->UnlockRect(0);
    }
    SafeRelease(&_back_y_texture);
    if(_back_u_texture) {
        _back_u_texture->UnlockRect(0);
    }
    SafeRelease(&_back_u_texture);
    if(_back_v_texture) {
        _back_v_texture->UnlockRect(0);
    }
    SafeRelease(&_back_v_texture);
    _texture_lock.Unlock();
}

void SourceTexture::clearRect()
{
    AutoLock auto_lock(&_rect_lock);
    memset(&_y_rect, 0, sizeof(_y_rect));
    memset(&_u_rect, 0, sizeof(_u_rect));
    memset(&_v_rect, 0, sizeof(_v_rect));
}

void SourceTexture::fillFrameData(char *buf_data,long buf_size, FRAME_INFO *frame_info)
{
    if(!buf_data || !frame_info || 0>=buf_size
       ||0==frame_info->nWidth*frame_info->nHeight*frame_info->nType) {
        return;
    }
	bool is_equal = _frame_width == frame_info->nWidth &&
					_frame_height == frame_info->nHeight &&
					_color_type == frame_info->nType;
    if(!is_equal) {
        releaseTexture();
        _frame_width = frame_info->nWidth;
        _frame_height = frame_info->nHeight;
        _color_type = frame_info->nType;
        createTexture();
    }

    switch(frame_info->nType)
    {
    case T_YV12:
        if(!fillYV12Texture(buf_data, buf_size)) {
            return;
        }
        break;
    case T_UYVY:
        if(!fillUYVYTexture(buf_data, buf_size)) {
            return;
        }
        break;
    case T_RGB32:
        if(!fillRGB32Texture(buf_data, buf_size)) {
            return;
        }
        break;
    default:
        return;
    }
    
    AutoLock auto_lock(&_texture_lock);
    IDirect3DTexture9 *swap_tex = nullptr;
    swap_tex = _front_y_texture;
    _front_y_texture = _back_y_texture;
    _back_y_texture = swap_tex;
    swap_tex = _front_u_texture;
    _front_u_texture = _back_u_texture;
    _back_u_texture = swap_tex;
    swap_tex = _front_v_texture;
    _front_v_texture = _back_v_texture;
    _back_v_texture = swap_tex;
    _is_ready = true;
}

bool SourceTexture::fillYV12Texture(char *buf_data, long buf_size)
{
    if(buf_size < 3*_frame_width*_frame_height/2) {
        return false;
    }
    if(!_y_rect.pBits || !_u_rect.pBits || !_v_rect.pBits) {
        return false;
    }

    buf_size = _frame_width * _frame_height;
    int dest_pitch = _y_rect.Pitch;
    char *dest_buffer = reinterpret_cast<char*>(_y_rect.pBits);
    AutoLock auto_lock(&_rect_lock);

    // Y component
    if(dest_pitch == _frame_width) {
        unaligned_memcpy_sse(dest_buffer, buf_data, buf_size);
        buf_data += buf_size;
    } else {
        for(int i=0; i<_frame_height; ++i) {
            unaligned_memcpy_sse(dest_buffer+i*dest_pitch, buf_data, _frame_width);
            buf_data += _frame_width;
        }
    }
    // U component
    buf_size /= 4;
    dest_pitch = _u_rect.Pitch;
    dest_buffer = reinterpret_cast<char*>(_u_rect.pBits);
    if(dest_pitch == _frame_width/2) {
        unaligned_memcpy_sse(dest_buffer, buf_data, buf_size);
        buf_data += buf_size;
    } else {
        for(int i=0; i<_frame_height/2; ++i) {
            unaligned_memcpy_sse(dest_buffer+i*dest_pitch, buf_data, _frame_width/2);
            buf_data += _frame_width/2;
        }
    }
    // V component
    dest_pitch = _v_rect.Pitch;
    dest_buffer = reinterpret_cast<char*>(_v_rect.pBits);
    if(dest_pitch == _frame_width/2) {
        unaligned_memcpy_sse(dest_buffer, buf_data, buf_size);
        buf_data += buf_size;
    } else {
        for(int i=0; i<_frame_height/2; ++i) {
            unaligned_memcpy_sse(dest_buffer+i*dest_pitch, buf_data, _frame_width/2);
            buf_data += _frame_width/2;
        }
    }

    return true;
}

bool SourceTexture::fillUYVYTexture(char *buf_data, long buf_size)
{
    if(buf_size < 2*_frame_width*_frame_height) {
        return false;
    }
    if(!_y_rect.pBits) {
        return false;
    }

    char *dest_buffer = reinterpret_cast<char*>(_y_rect.pBits);
    AutoLock auto_lock(&_rect_lock);

    // Y component
    if(_y_rect.Pitch == _frame_width*2) {
        unaligned_memcpy_sse(dest_buffer, buf_data, 2*_frame_width*_frame_height);
    } else {
        for(int i=0; i<_frame_height; ++i) {
            unaligned_memcpy_sse(dest_buffer+i*_y_rect.Pitch, buf_data, _frame_width*2);
            buf_data += _frame_width * 2;
        }
    }

    return true;
}

bool SourceTexture::fillRGB32Texture(char *buf_data, long buf_size)
{
    if(buf_size < 4*_frame_width*_frame_height) {
        return false;
    }
    if(!_y_rect.pBits) {
        return false;
    }

    char *dest_buffer = reinterpret_cast<char*>(_y_rect.pBits);
    AutoLock auto_lock(&_rect_lock);

    // Y component
    if(_y_rect.Pitch == _frame_width*4) {
        unaligned_memcpy_sse(dest_buffer, buf_data, 4*_frame_width*_frame_height);
    } else {
        for(int i=0; i<_frame_height; ++i) {
            unaligned_memcpy_sse(dest_buffer+i*_y_rect.Pitch, buf_data, _frame_width*4);
            buf_data += _frame_width * 4;
        }
    }

    return true;
}

//The reference count of Y texture increases 1.
IDirect3DTexture9 *SourceTexture::getYTexture()
{
    if(!_is_ready || !_front_y_texture) {
        return nullptr;
    }

    AutoLock auto_lock(&_texture_lock);
    _front_y_texture->AddRef();
    return _front_y_texture;
}

//The reference count of u texture increases 1.
IDirect3DTexture9 *SourceTexture::getUTexture()
{
    if(!_is_ready || !_front_u_texture) {
        return nullptr;
    }

    AutoLock auto_lock(&_texture_lock);
    _front_u_texture->AddRef();
    return _front_u_texture;
}

//The reference count of v texture increases 1.
IDirect3DTexture9 *SourceTexture::getVTexture()
{
    if(!_is_ready || !_front_v_texture) {
        return nullptr;
    }

    AutoLock auto_lock(&_texture_lock);
    _front_v_texture->AddRef();
    return _front_v_texture;
}