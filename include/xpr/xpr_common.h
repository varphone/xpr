#ifndef XPR_COMMON_H
#define XPR_COMMON_H

/// @defgroup xprcommon 通用定义
/// @brief     定义通用的宏，数据类型
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/12/1
///
/// @{
///

#ifndef XPR_AlignedUpTo
///
/// 将数值 v 向上对齐到 x
///
#  define XPR_AlignedUpTo(v, x)  (((v)+((x)-1)) & (~((x)-1)))
#endif

#ifndef XPR_MakeFourCC
/// @brief 合成 FourCC 编码
/// @param a    字节1
/// @param b    字节2
/// @param c    字节3
/// @param d    字节4
/// @return 32 位整数
#  define XPR_MakeFourCC( a, b, c, d ) \
     ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
     | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

#ifndef XPR_MakeVersion
///
/// 生成 32 位版本号
///
#  define XPR_MakeVersion(a, b, c) (a<<24|b<<16|c)
#endif

#ifndef _countof
///
/// 计算数组大小
///
#  define _countof(_array) (sizeof(_array) / sizeof(_array[0]))
#endif

#ifndef offsetof
///
/// 计算结构体成员偏移
///
#  define offsetof(st, m) __builtin_offsetof(st, m)
#endif

#ifndef XPR_FALSE
///
/// 布尔假值
///
#  define XPR_FALSE 0
#endif

#ifndef XPR_TRUE
///
/// 布尔真值
///
#  define XPR_TRUE 1
#endif

// FourCC
//==============================================================================
#ifndef AV_FOURCC_NULL
///
/// NULL
///
#  define AV_FOURCC_NULL XPR_MakeFourCC('N', 'U', 'L', 'L')
#endif

#ifndef AV_FOURCC_AAC
///
/// AAC audio codec
///
#  define AV_FOURCC_AAC XPR_MakeFourCC('m', 'p', '4', 'a')
#endif

#ifndef AV_FOURCC_G711A
///
/// G711.A audio codec
///
#  define AV_FOURCC_G711A XPR_MakeFourCC('7', '1', '1', 'A')
#endif

#ifndef AV_FOURCC_G711U
///
/// G711.U audio codec
///
#  define AV_FOURCC_G711U XPR_MakeFourCC('7', '1', '1', 'U')
#endif

#ifndef AV_FOURCC_JPEG
///
/// JPEG/MJPEG video codec
///
#  define AV_FOURCC_JPEG  XPR_MakeFourCC('J', 'P', 'E', 'G')
#endif

#ifndef AV_FOURCC_H264
///
/// H264 video codec
///
#  define AV_FOURCC_H264  XPR_MakeFourCC('a', 'v', 'c', '1')
#endif

#ifndef AV_FOURCC_HKMI
///
/// Hikvision media information data codec
///
#  define AV_FOURCC_HKMI  XPR_MakeFourCC('H', 'K', 'M', 'I')
#endif

#ifndef AV_FOURCC_MP2P
///
/// MPEG2-PS data codec
///
#  define AV_FOURCC_MP2P  XPR_MakeFourCC('M', 'P', '2', 'P')
#endif

#ifndef AV_FOURCC_MP2T
///
/// MPEG-TS data codec
///
#  define AV_FOURCC_MP2T  XPR_MakeFourCC('M', 'P', '2', 'T')
#endif

#ifndef AV_FOURCC_PCMA
///
/// PCM-alaw audio codec
///
#  define AV_FOURCC_PCMA XPR_MakeFourCC('P', 'C', 'M', 'A')
#endif

#ifndef AV_FOURCC_PCMU
///
/// PCM-ulaw audio codec
///
#  define AV_FOURCC_PCMU XPR_MakeFourCC('P', 'C', 'M', 'U')
#endif

#ifndef AV_FOURCC_S16B
///
/// Signed 16 bit PCM big-endian audio codec
///
#  define AV_FOURCC_S16B XPR_MakeFourCC('S', '1', '6', 'B')
#endif

#ifndef AV_FOURCC_S16L
///
/// Signed 16 bit PCM little-endian audio codec
///
#  define AV_FOURCC_S16L XPR_MakeFourCC('S', '1', '6', 'L')
#endif

#ifndef AV_FOURCC_MDSD
///
/// Motion detection summy data codec
///
#  define AV_FOURCC_MDSD XPR_MakeFourCC('M', 'D', 'S', 'D')
#endif

#ifndef AV_FOURCC_XDMI
///
/// Xian Dian media information data codec
///
#  define AV_FOURCC_XDMI XPR_MakeFourCC('X', 'D', 'M', 'I')
#endif

#ifndef AV_FOURCC_XPMI
///
/// X Portable media information data codec
///
#  define AV_FOURCC_XPMI XPR_MakeFourCC('X', 'P', 'M', 'I')
#endif

/// @}
///

#endif // XPR_COMMON_H

