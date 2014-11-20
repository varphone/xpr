#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d9.h>
#include "autolock.hpp"

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN(TypeName)
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);               \
    void operator=(const TypeName&)
#endif

struct FRAME_INFO;
class SourceTexture
{
private:
    DISALLOW_COPY_AND_ASSIGN(SourceTexture);
    SourceTexture(IDirect3DDevice9Ex *device);
public:
    ~SourceTexture();
    static SourceTexture *createSourceTexture(IDirect3DDevice9Ex *device);
    void fillFrameData(char *buf_data, long buf_size, FRAME_INFO *frame_info);
    IDirect3DTexture9 *getYTexture();
    IDirect3DTexture9 *getUTexture();
    IDirect3DTexture9 *getVTexture();

private:
    bool createTexture();
    bool createYV12Texture();
    bool createUYVYTexture();
    bool createRGB32Texture();
    void releaseTexture();
    void clearRect();
    bool fillYV12Texture(char *buf_data, long buf_size);
    bool fillUYVYTexture(char *buf_data, long buf_size);
    bool fillRGB32Texture(char *buf_data, long buf_size);

private:
    IDirect3DDevice9Ex *_device;
    IDirect3DTexture9 *_front_y_texture;
    IDirect3DTexture9 *_front_u_texture;
    IDirect3DTexture9 *_front_v_texture;
    IDirect3DTexture9 *_back_y_texture;
    IDirect3DTexture9 *_back_u_texture;
    IDirect3DTexture9 *_back_v_texture;
    D3DLOCKED_RECT _y_rect;
    D3DLOCKED_RECT _u_rect;
    D3DLOCKED_RECT _v_rect;
    long _frame_width;
    long _frame_height;
    long _color_type;
    bool _is_ready;
    SectionLock _texture_lock;
    SectionLock _rect_lock;
};