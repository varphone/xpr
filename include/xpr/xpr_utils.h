#ifndef XPR_UTILS_H
#define XPR_UTILS_H

#include <stdint.h>
#include <wchar.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define DBG_L0  0
#define DBG_L1  1
#define DBG_L2  2
#define DBG_L3  3
#define DBG_L4  4
#define DBG_L5  5
#define DBG_L6  6
#define DBG_L7  7
#define DBG_L8  8
#define DBG_L9  9

#ifdef DBG_LEVEL
#define ASSERT(x)       do { if (!(x)) debug_break(); } while (0)
#define ASSERT_PTR(x)   do { if (!(x) || !(((uintptr_t)(x)) & 0xffff0000)) debug_break(); } while (0)
#define DBG dbg_printf
#else
#define ASSERT(x)                   do { } while (0)
#define ASSERT_PTR(x)               do { } while (0)
#define DBG(level, format, ...)     do { } while (0)
#endif

#define swap16(x)       ((((x) & 0xFF00)>>8) | (((x) & 0x00FF)<<8))
#define swap32(x)       ((((x) & 0xFF000000)>>24) | \
                         (((x) & 0x00FF0000)>>8)  | \
                         (((x) & 0x0000FF00)<<8)  | \
                         (((x) & 0x000000FF)<<24))

#if !defined(WIN32)
#define closesocket(x) close(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void dbg_printf(int level, char* format, ...);

char* skip_blank(char* s);

/// 分离 Key/Value 格式字符串.
/// @param [in] line        Key/Value 格式的字符串
/// @param [in] key         用于接收 Key 指针的缓冲区
/// @param [in] value       用于接收 Value 指针的缓冲区
/// @retval 0   成功
/// @retval -1  失败
int split_to_kv(char* line, char** key, char** value);

#if defined(_MSC_VER)
#define aligned_free    _aligned_free
#define aligned_malloc  _aligned_malloc
#define debug_break     DebugBreak
#define snprintf        sprintf_s
char* strsep(char** stringp, const char* delim);
#endif
#if !defined(_MSC_VER)
#define aligned_free            free
#define aligned_malloc(x,v)     malloc(x)
#define debug_break             abort
int strcpy_s(char* strDestination, size_t numberOfElements, const char* strSource);
#endif

char* trim_all(char* s);
char* trim_quotes(char* s);
char* trim_tailer(char* s);

int calc_lines(const char* s);
const char* get_next_line(const char** sp);

// IntRange
//==============================================================================
#ifndef XPR_INTRAGE_TYPE_DEFINED
#define XPR_INTRAGE_TYPE_DEFINED
struct XPR_IntRange {
    int min;
    int max;
};
typedef struct XPR_IntRange XPR_IntRange;
#endif // XPR_INTRAGE_TYPE_DEFINED

XPR_IntRange XPR_IntRangeParse(const char* s);
int XPR_IntRangePrint(XPR_IntRange rng, char* s);
char* XPR_IntRangeToString(XPR_IntRange rng);

// Rect
//==============================================================================
#ifndef XPR_RECT_TYEP_DEFINED
#define XPR_RECT_TYEP_DEFINED
typedef struct XPR_Rect {
    int left;
    int top;
    int right;
    int bottom;
} XPR_Rect;
#endif // XPR_RECT_TYEP_DEFINED

// PackBits
//==============================================================================
/// @brief PackBits encode
/// @param [in] data    Original data
/// @param [in] length  Original data length
/// @param [in] buffer  Buffer to save packed data
/// @param [in] size    Buffer size
/// @return bytes of packed data
int XPR_PackBits(unsigned char* data, int length,
                 unsigned char* buffer, int size);

/// @brief PackBits decode
/// @param [in] data    Packed data
/// @param [in] length  Packed data length
/// @param [in] buffer  Buffer to save unpacked data
/// @param [in] size    Buffer size
/// @return bytes of unpacked data
int XPR_UnPackBits(unsigned char* data, int length,
                   unsigned char* buffer, int size);

// Rect
//==============================================================================
#ifndef XPR_RECT_TYEP_DEFINED
#define XPR_RECT_TYEP_DEFINED
typedef struct XPR_Rect {
    int left;
    int top;
    int right;
    int bottom;
} XPR_Rect;
#endif // XPR_RECT_TYEP_DEFINED

// Uri
//==============================================================================

/// @brief URI 编码
/// @param [in] uri         要进行 URI 编码的字符串
/// @param [in] length      要进行 URI 编码的字符串字节数, -1 为自动检测
/// @param [in,out] buffer  接收 URI 编码后的字符串的缓冲区, 可以为 NULL
/// @param [in,out] size    接收 URI 编码后的字符串的缓冲区长度, 且用于返回编码后的字符串字节数
/// @return 返回编码后的字符串地址, 当 buffer = NULL 时, 返回的是动态分配的内存地址
/// @note 当使用 buffer = NULL 方式的返回值时, 需要在返回值不再使用调用 XPR_Free() 来释放其所分配的资源
int8_t* XPR_UriEncode(const uint8_t* uri, int length, int8_t* buffer, int* size);

/// @brief URI 解码
/// @param [in] uri         要进行 URI 解码的字符串
/// @param [in] length      要进行 URI 解码的字符串字节数, -1 为自动检测
/// @param [in,out] buffer  接收 URI 解码后的字符串的缓冲区, 可以为 NULL
/// @param [in,out] size    接收 URI 解码后的字符串的缓冲区长度, 且用于返回解码后的字符串字节数
/// @return 返回解码后的字符串地址, 当 buffer = NULL 时, 返回的是动态分配的内存地址
/// @note 当使用 buffer = NULL 方式的返回值时, 需要在返回值不再使用调用 XPR_Free() 来释放其所分配的资源
uint8_t* XPR_UriDecode(const int8_t* uri, int length, uint8_t* buffer, int* size);

// Unicode
//==============================================================================
uint16_t* XPR_UTF8_UTF16(const uint8_t* utf8, int length, uint16_t* utf16, int* size);

uint16_t* XPR_UTF8_UTF16BE(const uint8_t* utf8, int length, uint16_t* utf16, int* size);

uint16_t* XPR_UTF8_UTF16LE(const uint8_t* utf8, int length, uint16_t* utf16, int* size);

uint32_t* XPR_UTF8_UTF32(const uint8_t* utf8, int length, uint32_t* utf32, int* size);

uint32_t* XPR_UTF8_UTF32BE(const uint8_t* utf8, int length, uint32_t* utf32, int* size);

uint32_t* XPR_UTF8_UTF32LE(const uint8_t* utf8, int length, uint32_t* utf32, int* size);

int XPR_UTF8_GetChars(const uint8_t* utf8, int length);

wchar_t* XPR_UTF8_Unicode(const uint8_t* utf8, int length, wchar_t* wcs, int* size);

uint8_t* XPR_UTF16_UTF8(const uint16_t* utf16, int length, uint8_t* utf8, int size);

uint8_t* XPR_UTF16BE_UTF8(const uint16_t* utf16, int length, uint8_t* utf8, int size);

uint8_t* XPR_UTF16LE_UTF8(const uint16_t* utf16, int length, uint8_t* utf8, int size);

uint8_t* XPR_UTF32_UTF8(const uint32_t* utf32, int length, uint8_t* utf8, int size);

uint8_t* XPR_UTF32BE_UTF8(const uint32_t* utf32, int length, uint8_t* utf8, int size);

uint8_t* XPR_UTF32LE_UTF8(const uint32_t* utf32, int length, uint8_t* utf8, int size);

uint8_t* XPR_Unicode_UTF8(const wchar_t* wcs, int length, uint8_t* utf8, int size);

// Byte Order Marker
//==============================================================================
extern uint8_t XPR_UTF8_BOM[3];
extern uint8_t XPR_UTF16BE_BOM[2];
extern uint8_t XPR_UTF16LE_BOM[2];
extern uint8_t XPR_UTF32BE_BOM[4];
extern uint8_t XPR_UTF32LE_BOM[4];

// Template
//=============================================================================
struct XPR_Template;
typedef struct XPR_Template XPR_Template;

typedef int (*XPR_TemplateHandler)(XPR_Template* tmpl, const char* var);

XPR_Template* XPR_TemplateNew(void);
int XPR_TemplateDestroy(XPR_Template* tmpl);

int XPR_TemplateBuild(XPR_Template* tmpl);
int XPR_TemplateLoad(XPR_Template* tmp, const char* file);

int XPR_TemplateSetHandler(XPR_Template* tmpl, XPR_TemplateHandler cb);
XPR_TemplateHandler XPR_TemplateGetHandler(XPR_Template* tmpl);

int XPR_TemplateSetOpaque(XPR_Template* tmpl, void* opaque);
void* XPR_TemplateGetOpaque(XPR_Template* tmpl);

char* XPR_TemplateGetData(XPR_Template* tmpl);
int XPR_TemplateGetDataSize(XPR_Template* tmpl);

int XPR_TemplatePutf(XPR_Template* tmpl, const char* fmt, ...);
int XPR_TemplatePutData(XPR_Template* tmpl, char* data, int length);

char* XPR_TemplateGetSpace(XPR_Template* tmpl);
int XPR_TemplateGetSpaceSize(XPR_Template* tmpl);

#ifdef __cplusplus
}
#endif

#endif // XPR_UTILS_H