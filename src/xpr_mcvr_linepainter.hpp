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

struct IDirect3DDevice9;
struct IDirect3DIndexBuffer9;
struct IDirect3DVertexBuffer9;
class LinePainter
{
private:
    DISALLOW_COPY_AND_ASSIGN(LinePainter);
    LinePainter(void){}

public:
    LinePainter(IDirect3DDevice9 *device, HWND hwnd, int rows, int cols);
    ~LinePainter(void);
    void resetDrawer(int rows, int cols);
    void drawAllLines(const int cur_port);
    void drawFullLines(const bool is_highlight);
    void onLostDevice(void);

private:
    bool initDrawer(const int rows, const int cols);
    bool updateCurrentVertex(const int cur_port);

private:
    IDirect3DDevice9 *_device;
    HWND _hwnd;
    int _rows_count;
    int _cols_count;
    int _cur_port;
    IDirect3DVertexBuffer9 *_vertex_buffer;
    IDirect3DVertexBuffer9 *_current_vertex;
};