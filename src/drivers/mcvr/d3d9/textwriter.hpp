#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// A macro to disallow the copy constructor and operator= functions 
// This should be used in the private:declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);         \
    TypeName& operator=(const TypeName&)
#endif

struct ID3DXFont;
struct IDirect3DDevice9;
class TextWriter
{
private:
    DISALLOW_COPY_AND_ASSIGN(TextWriter);
public:
    explicit TextWriter(IDirect3DDevice9 *device);
    ~TextWriter(void);
    void initTextWriter(void);
    void releaseTextWriter(void);
    void onLostDevice(void);
    void onResetDevice(void);
    void drawTitle(RECT rect, const char *text);
    void drawState(RECT rect, const char *text);

private:
    IDirect3DDevice9 *_device;
    ID3DXFont *_back_drawer;
    ID3DXFont *_title_font;
    ID3DXFont *_state_font;
};