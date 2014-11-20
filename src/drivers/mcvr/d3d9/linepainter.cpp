#include <d3d9.h>
#include <d3dx9.h>
#include <atlbase.h>
#include <xpr/xpr_mcvr.h>
#include "linepainter.hpp"
#include "safefree.hpp"

//线条顶点
#define D3DFVF_VERTEX_LINE (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
struct CUSTOMVERTEX_LINE {
    D3DXVECTOR3 pos;
    float rhw;
    DWORD color;
};
//顶点颜色
const DWORD kBlackColor     = 0xFF000000;
const DWORD kHighLightColor = 0xFFFFFF00;

LinePainter::LinePainter(IDirect3DDevice9 *device, HWND hwnd, int rows, int cols)
    :_device(device)
    ,_hwnd(hwnd)
    ,_rows_count(rows)
    ,_cols_count(cols)
    ,_cur_port(0)
    ,_vertex_buffer(nullptr)
    ,_current_vertex(nullptr)
{
    initDrawer(_rows_count, _cols_count);
}

LinePainter::~LinePainter()
{
    SafeRelease(&_vertex_buffer);
    SafeRelease(&_current_vertex);
}

bool LinePainter::initDrawer(const int rows, const int cols)
{
    if(!_device || !_hwnd || 0>=rows*cols) {
        return false;
    }
    
    UINT count = 2 * (rows + 1 + cols + 1) + 5 + 5;
    CUSTOMVERTEX_LINE *vertices = new CUSTOMVERTEX_LINE[count];
    if(!vertices) {
        return false;
    }
    memset(vertices, 0, sizeof(CUSTOMVERTEX_LINE)*count);
    RECT rect;
    GetClientRect(_hwnd, &rect);
    float left = static_cast<float>(rect.left);
    float width = static_cast<float>(rect.right - rect.left);
    float top = static_cast<float>(rect.top);
    float height = static_cast<float>(rect.bottom - rect.top);
    //竖线
    for(int i=0; i<cols+1;++i) {
        vertices[2*i+0].pos.x = left + i * width / static_cast<float>(cols);
        vertices[2*i+0].pos.y = top;
        vertices[2*i+1].pos.x = vertices[2*i+0].pos.x;
        vertices[2*i+1].pos.y = static_cast<float>(rect.bottom);
    }
    vertices[2*cols+0].pos.x = static_cast<float>(rect.right) - 1.0f;
    vertices[2*cols+1].pos.x = static_cast<float>(rect.right)- 1.0f;
    //横线
    int offset = 2 * (cols+1);
    for(int i=0; i<rows+1; ++i) {
        vertices[offset+2*i+0].pos.x = left;
        vertices[offset+2*i+0].pos.y = top + i * height / static_cast<float>(rows);
        vertices[offset+2*i+1].pos.x = static_cast<float>(rect.right);
        vertices[offset+2*i+1].pos.y = vertices[offset+2*i+0].pos.y;
    }
    vertices[offset+2*rows+0].pos.y = static_cast<float>(rect.bottom-1);
    vertices[offset+2*rows+1].pos.y = static_cast<float>(rect.bottom-1);
    //黑色大边框
    offset += 2 * (rows+1);
    vertices[offset+0].pos.x = left;
    vertices[offset+0].pos.y = top;
    vertices[offset+1].pos.x = static_cast<float>(rect.right-1);
    vertices[offset+1].pos.y = top;
    vertices[offset+2].pos.x = static_cast<float>(rect.right-1);
    vertices[offset+2].pos.y = static_cast<float>(rect.bottom-1);
    vertices[offset+3].pos.x = left;
    vertices[offset+3].pos.y = static_cast<float>(rect.bottom-1);
    vertices[offset+4].pos   = vertices[offset+0].pos;
    //高亮大边框
    for(int i=0; i<5; ++i) {
        vertices[offset+5+i].pos = vertices[offset+i].pos;
    }
    //所有顶点
    for(UINT i=0; i<count; ++i) {
        vertices[i].pos.z = 0.0f;
        vertices[i].rhw   = 1.0f;
        vertices[i].color = kBlackColor;
    }
    //高亮顶点
    for(int i=0; i<5; ++i) {
        vertices[count-5+i].color = kHighLightColor;
    }

    HRESULT hr = S_OK;
    UINT len = sizeof(CUSTOMVERTEX_LINE) * count;
    CComPtr<IDirect3DVertexBuffer9> vertex_buffer;
    hr = _device->CreateVertexBuffer(
        len, D3DUSAGE_WRITEONLY,
        D3DFVF_VERTEX_LINE, D3DPOOL_DEFAULT,
        &vertex_buffer, nullptr);
    if(FAILED(hr)) {
        SafeDeleteArray(&vertices);
        return false;
    }
    void *data = nullptr;
    hr = vertex_buffer->Lock(0, 0, &data, D3DLOCK_NOSYSLOCK);
    if(FAILED(hr)) {
        SafeDeleteArray(&vertices);
        return false;
    }
    memcpy_s(data, len, vertices, len);
    vertex_buffer->Unlock();
    SafeDeleteArray(&vertices);
    SafeRelease(&_vertex_buffer);
    vertex_buffer.CopyTo(&_vertex_buffer);

    if(!_current_vertex) {
        len = 5 * sizeof(CUSTOMVERTEX_LINE);
        hr = _device->CreateVertexBuffer(
            len, D3DUSAGE_DYNAMIC,
            D3DFVF_VERTEX_LINE, D3DPOOL_DEFAULT,
            &_current_vertex, nullptr);
        if(FAILED(hr)) {
            return false;
        }
    }

    return true;
}

void LinePainter::onLostDevice()
{
    SafeRelease(&_vertex_buffer);
    SafeRelease(&_current_vertex);
}

void LinePainter::resetDrawer(int rows, int cols)
{
    if(initDrawer(rows, cols)) {
        _rows_count = rows;
        _cols_count = cols;
        int cur_port = _cur_port;
        _cur_port = 0;
        updateCurrentVertex(cur_port);
    }
    _cur_port = 0;
}

void LinePainter::drawAllLines(const int cur_port)
{
    if(!_device || !_vertex_buffer) {
        return;
    }
    //所有边框
    HRESULT hr = S_OK;
    hr = _device->SetFVF(D3DFVF_VERTEX_LINE);
    hr = _device->SetStreamSource(0, _vertex_buffer, 0, sizeof(CUSTOMVERTEX_LINE));
    hr = _device->DrawPrimitive(D3DPT_LINELIST, 0, _rows_count+1+_cols_count+1);
    //选中边框
    if(0<cur_port && cur_port<=_rows_count*_cols_count) {
        if(updateCurrentVertex(cur_port)) {
            _device->SetStreamSource(0, _current_vertex, 0, sizeof(CUSTOMVERTEX_LINE));
            _device->DrawPrimitive(D3DPT_LINESTRIP, 0, 4);
        }
    }
}

void LinePainter::drawFullLines(const bool is_highlight)
{
    if(!_device || !_vertex_buffer) {
        return;
    }

    _device->SetFVF(D3DFVF_VERTEX_LINE);
    UINT offset = 2 * (_rows_count + 1 + _cols_count + 1) * sizeof(CUSTOMVERTEX_LINE);
    if(is_highlight) {
        offset += 5 * sizeof(CUSTOMVERTEX_LINE);
    }
    _device->SetStreamSource(0, _vertex_buffer, offset, sizeof(CUSTOMVERTEX_LINE));
    _device->DrawPrimitive(D3DPT_LINESTRIP, 0, 4);
}

bool LinePainter::updateCurrentVertex(const int cur_port)
{
    if(!_current_vertex) {
        return false;
    }
    if(cur_port == _cur_port) {
        return true;
    }
    _cur_port = cur_port;

    RECT rect;
    GetClientRect(_hwnd, &rect);
    float left = static_cast<float>(rect.left);
    float width = static_cast<float>(rect.right-rect.left) / static_cast<float>(_cols_count);
    float top = static_cast<float>(rect.top);
    float height = static_cast<float>(rect.bottom-rect.top) / static_cast<float>(_rows_count);
    CUSTOMVERTEX_LINE vertices[5];
    memset(vertices, 0, 5*sizeof(CUSTOMVERTEX_LINE));
    vertices[0].pos.x = left + static_cast<float>((_cur_port-1)%_cols_count)*width;
    vertices[0].pos.y = top + static_cast<float>((_cur_port-1)/_cols_count)*height;
    vertices[1].pos.x = vertices[0].pos.x + width;
    vertices[1].pos.y = vertices[0].pos.y;
    vertices[2].pos.x = vertices[1].pos.x;
    vertices[2].pos.y = vertices[1].pos.y + height;
    vertices[3].pos.x = vertices[0].pos.x;
    vertices[3].pos.y = vertices[2].pos.y;
    vertices[4].pos = vertices[0].pos;
    for(int i=0; i<5; ++i) {
        vertices[i].pos.z = 0.0f;
        vertices[i].rhw = 1.0f;
        vertices[i].color = kHighLightColor;
    }
    if(0 == cur_port%_cols_count) {
        vertices[1].pos.x -= 1.0f;
        vertices[2].pos.x -= 1.0f;
    }
    if((_rows_count-1) <= (cur_port-1)/_cols_count) {
        vertices[2].pos.y -= 1.0f;
        vertices[3].pos.y -= 1.0f;
    }

    HRESULT hr = S_OK;
    void *data;
    hr = _current_vertex->Lock(0, 0, &data, D3DLOCK_NOSYSLOCK);
    if(FAILED(hr)) {
        return false;
    }
    memcpy_s(data, 5*sizeof(CUSTOMVERTEX_LINE), vertices, 5*sizeof(CUSTOMVERTEX_LINE));
    _current_vertex->Unlock();

    return true;
}
