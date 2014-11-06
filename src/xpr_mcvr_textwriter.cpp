#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdio>
#include "xpr_mcvr_safefree.hpp"
#include "xpr_mcvr_textwriter.hpp"

static const D3DCOLOR kTextColor = D3DCOLOR_XRGB(255, 255, 0);
static const D3DCOLOR kBackColor = D3DCOLOR_ARGB(255, 40, 40, 40);

TextWriter::TextWriter(IDirect3DDevice9 *device)
    :_device(device)
    ,_back_drawer(nullptr)
    ,_title_font(nullptr)
    ,_state_font(nullptr)
{
    initTextWriter();
}

TextWriter::~TextWriter()
{
    releaseTextWriter();
}

void TextWriter::initTextWriter()
{
    if(!_device) {
        return;
    }

    D3DXFONT_DESCW font_desc;
    memset(&font_desc, 0, sizeof(font_desc));
    font_desc.Weight = 500;
    font_desc.MipLevels = 0;
    font_desc.Italic = FALSE;
    font_desc.CharSet = DEFAULT_CHARSET;
    font_desc.OutputPrecision = OUT_STRING_PRECIS;
    font_desc.Quality = CLEARTYPE_QUALITY;
    font_desc.PitchAndFamily = DEFAULT_PITCH;// | FF_DONTCARE;

    font_desc.Height = 12;
    font_desc.Width = 6;
    swprintf_s(font_desc.FaceName, L"MS Gothic");
    D3DXCreateFontIndirectW(_device, &font_desc, &_back_drawer);
    swprintf_s(font_desc.FaceName, L"ËÎÌå");
    D3DXCreateFontIndirectW(_device, &font_desc, &_title_font);
    
    swprintf_s(font_desc.FaceName, L"ËÎÌå");
    font_desc.Height = 30;
    font_desc.Width = 15;
    D3DXCreateFontIndirectW(_device, &font_desc, &_state_font);
}

void TextWriter::releaseTextWriter()
{
    SafeRelease(&_back_drawer);
    SafeRelease(&_title_font);
    SafeRelease(&_state_font);
}

void TextWriter::onLostDevice()
{
    if(_back_drawer) {
        _back_drawer->OnLostDevice();
    }
    if(_title_font) {
        _title_font->OnLostDevice();
    }
    if(_state_font) {
        _state_font->OnLostDevice();
    }
}

void TextWriter::onResetDevice()
{
    if(_back_drawer) {
        _back_drawer->OnResetDevice();
    }
    if(_title_font) {
        _title_font->OnResetDevice();
    }
    if(_state_font) {
        _state_font->OnResetDevice();
    }
}

void TextWriter::drawTitle(RECT rect, const char *text)
{
    if(_back_drawer && _title_font) {
        int len = strlen(text);
        char back_color[100] = {'\0'};
        for(int i=0; i<len; ++i) {
            strcat_s(back_color, "¨€");
        }
        _back_drawer->DrawTextA(nullptr, back_color, -1, &rect, DT_LEFT | DT_TOP, kBackColor);
        _title_font->DrawTextA(nullptr, text, -1, &rect, DT_LEFT | DT_TOP, kTextColor);
    }
}

void TextWriter::drawState(RECT rect, const char *text)
{
    if(_state_font) {
        _state_font->DrawTextA(nullptr, text, -1, &rect, DT_CENTER | DT_VCENTER, kTextColor);
    }
}
