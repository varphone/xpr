#ifndef XPR_UTILS_H
#define XPR_UTILS_H

#include <stdint.h>

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

// H264 Utilities
//==============================================================================
#define XPR_H264_NALU_AUD    0x09
#define XPR_H264_NALU_SEI    0x06
#define XPR_H264_NALU_SPS    0x07
#define XPR_H264_NALU_PPS    0x08
#define XPR_H264_NALU_I_SLC  0x05
#define XPR_H264_NALU_P_SLC  0x01

#ifndef XPR_H264NALUINFO_TYPE_DEFINED
#define XPR_H264NALUINFO_TYPE_DEFINED
struct XPR_H264NaluInfo;
typedef struct XPR_H264NaluInfo XPR_H264NaluInfo;
#endif // XPR_H264NALUINFO_TYPE_DEFINED

struct XPR_H264NaluInfo {
    const uint8_t* data;
    unsigned int length;
};

int XPR_ScanH264Nalus(const uint8_t* data, unsigned int length,
                      XPR_H264NaluInfo nalus[], unsigned int maxNalus);

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

#ifdef __cplusplus
}
#endif

#endif // XPR_UTILS_H

