/*
 * File: xpr_utils.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 杂项辅助接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:28:28 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_UTILS_H
#define XPR_UTILS_H

#include <stdint.h>
#include <wchar.h>
#include <xpr/xpr_common.h>

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

#if (defined(DEBUG) || defined(_DEBUG)) && !defined(DBG_LEVEL)
#define DBG_LEVEL       DBG_L5
#endif

#ifdef DBG_LEVEL
#define ASSERT(x)       do { if (!(x)) xpr_debug_break(); } while (0)
#define ASSERT_PTR(x)   do { if (!(x) || !(((uintptr_t)(x)) & 0xffff0000)) xpr_debug_break(); } while (0)
#define DBG             xpr_dbg_printf
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

#if defined(_MSC_VER)
#define aligned_free            _aligned_free
#define aligned_malloc          _aligned_malloc
#define snprintf                sprintf_s
#define xpr_debug_break         DebugBreak
char* strsep(char** stringp, const char* delim);
#endif
#if !defined(_MSC_VER)
#define aligned_free            free
#define aligned_malloc(x,v)     malloc(x)
#define xpr_debug_break         abort
int strcpy_s(char* strDestination, size_t numberOfElements, const char* strSource);
#endif

XPR_API void xpr_dbg_printf(int level, const char* format, ...);
XPR_API int xpr_dbg_get_level(void);
XPR_API void xpr_dbg_set_level(int level);

XPR_API char* xpr_skip_blank(char* s);

/// 分离 Key/Value 格式字符串.
/// @param [in] line        Key/Value 格式的字符串
/// @param [in] key         用于接收 Key 指针的缓冲区
/// @param [in] value       用于接收 Value 指针的缓冲区
/// @retval 0   成功
/// @retval -1  失败
XPR_API int xpr_split_to_kv(char* line, char** key, char** value);

XPR_API char* xpr_trim_all(char* s);
XPR_API char* xpr_trim_quotes(char* s);
XPR_API char* xpr_trim_tailer(char* s);

XPR_API int xpr_calc_lines(const char* s);
XPR_API const char* xpr_get_next_line(const char** sp);

/// 切割字符串并调用回调函数
/// @param [in] str		要切割的字符串
/// @param [in] length	要切割的字符长度
/// @param [in] delim	字符串分隔符
/// @param [in] filter  回调函数
/// @param [in] opaque  回调函数关联数据
/// return 无
XPR_API void xpr_foreach_s(const char* str, int length, const char* delim,
                           void (*filter)(void* opaque, char* segment),
                           void* opaque);

/// Convert "1234x1234" or "1234X1234" formated string into with and height.
/// Return XPR_ERR_OK on success, others error.
XPR_API int xpr_s2res(const char* str, int* width, int* height);

/// Convert "1.0,2.0" or "[2.0,3.0]" formated string into a double array.
/// Return XPR_ERR_OK on success, others error.
XPR_API int xpr_s2dvec2(const char* str, double vec2[2]);

/// Convert "1.0,2.0" or "[2.0,3.0]" formated string into a float array.
/// Return XPR_ERR_OK on success, others error.
XPR_API int xpr_s2fvec2(const char* str, float vec2[2]);

/// Convert "1,2" or "[2,3]" formated string into a int array.
/// Return XPR_ERR_OK on success, others error.
XPR_API int xpr_s2ivec2(const char* str, int vec2[2]);

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

XPR_API XPR_IntRange XPR_IntRangeParse(const char* s);
XPR_API int XPR_IntRangePrint(XPR_IntRange rng, char* s);
XPR_API char* XPR_IntRangeToString(XPR_IntRange rng);

// PackBits
//==============================================================================
/// @brief PackBits encode
/// @param [in] data    Original data
/// @param [in] length  Original data length
/// @param [in] buffer  Buffer to save packed data
/// @param [in] size    Buffer size
/// @return bytes of packed data
XPR_API int XPR_PackBits(unsigned char* data, int length, unsigned char* buffer,
                         int size);

/// @brief PackBits decode
/// @param [in] data    Packed data
/// @param [in] length  Packed data length
/// @param [in] buffer  Buffer to save unpacked data
/// @param [in] size    Buffer size
/// @return bytes of unpacked data
XPR_API int XPR_UnPackBits(unsigned char* data, int length,
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
XPR_API int8_t* XPR_UriEncode(const uint8_t* uri, int length, int8_t* buffer,
                              int* size);

/// @brief URI 解码
/// @param [in] uri         要进行 URI 解码的字符串
/// @param [in] length      要进行 URI 解码的字符串字节数, -1 为自动检测
/// @param [in,out] buffer  接收 URI 解码后的字符串的缓冲区, 可以为 NULL
/// @param [in,out] size    接收 URI 解码后的字符串的缓冲区长度, 且用于返回解码后的字符串字节数
/// @return 返回解码后的字符串地址, 当 buffer = NULL 时, 返回的是动态分配的内存地址
/// @note 当使用 buffer = NULL 方式的返回值时, 需要在返回值不再使用调用 XPR_Free() 来释放其所分配的资源
XPR_API uint8_t* XPR_UriDecode(const int8_t* uri, int length, uint8_t* buffer,
                               int* size);

// Unicode
//==============================================================================
XPR_API uint16_t* XPR_UTF8_UTF16(const uint8_t* utf8, int length,
                                 uint16_t* utf16, int* size);

XPR_API uint16_t* XPR_UTF8_UTF16BE(const uint8_t* utf8, int length,
                                   uint16_t* utf16, int* size);

XPR_API uint16_t* XPR_UTF8_UTF16LE(const uint8_t* utf8, int length,
                                   uint16_t* utf16, int* size);

XPR_API uint32_t* XPR_UTF8_UTF32(const uint8_t* utf8, int length,
                                 uint32_t* utf32, int* size);

XPR_API uint32_t* XPR_UTF8_UTF32BE(const uint8_t* utf8, int length,
                                   uint32_t* utf32, int* size);

XPR_API uint32_t* XPR_UTF8_UTF32LE(const uint8_t* utf8, int length,
                                   uint32_t* utf32, int* size);

XPR_API int XPR_UTF8_GetChars(const uint8_t* utf8, int length);

XPR_API wchar_t* XPR_UTF8_Unicode(const uint8_t* utf8, int length, wchar_t* wcs,
                                  int* size);

XPR_API uint8_t* XPR_UTF16_UTF8(const uint16_t* utf16, int length,
                                uint8_t* utf8, int size);

XPR_API uint8_t* XPR_UTF16BE_UTF8(const uint16_t* utf16, int length,
                                  uint8_t* utf8, int size);

XPR_API uint8_t* XPR_UTF16LE_UTF8(const uint16_t* utf16, int length,
                                  uint8_t* utf8, int size);

XPR_API uint8_t* XPR_UTF32_UTF8(const uint32_t* utf32, int length,
                                uint8_t* utf8, int size);

XPR_API uint8_t* XPR_UTF32BE_UTF8(const uint32_t* utf32, int length,
                                  uint8_t* utf8, int size);

XPR_API uint8_t* XPR_UTF32LE_UTF8(const uint32_t* utf32, int length,
                                  uint8_t* utf8, int size);

XPR_API uint8_t* XPR_Unicode_UTF8(const wchar_t* wcs, int length, uint8_t* utf8,
                                  int size);

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

XPR_API XPR_Template* XPR_TemplateNew(void);
XPR_API int XPR_TemplateDestroy(XPR_Template* tmpl);

XPR_API int XPR_TemplateBuild(XPR_Template* tmpl);
XPR_API int XPR_TemplateLoad(XPR_Template* tmp, const char* file);

XPR_API int XPR_TemplateSetHandler(XPR_Template* tmpl, XPR_TemplateHandler cb);
XPR_API XPR_TemplateHandler XPR_TemplateGetHandler(XPR_Template* tmpl);

XPR_API int XPR_TemplateSetOpaque(XPR_Template* tmpl, void* opaque);
XPR_API void* XPR_TemplateGetOpaque(XPR_Template* tmpl);

XPR_API char* XPR_TemplateGetData(XPR_Template* tmpl);
XPR_API int XPR_TemplateGetDataSize(XPR_Template* tmpl);

XPR_API int XPR_TemplatePutf(XPR_Template* tmpl, const char* fmt, ...);
XPR_API int XPR_TemplatePutData(XPR_Template* tmpl, char* data, int length);

XPR_API char* XPR_TemplateGetSpace(XPR_Template* tmpl);
XPR_API int XPR_TemplateGetSpaceSize(XPR_Template* tmpl);

#ifdef __cplusplus
}
#endif

#endif // XPR_UTILS_H
