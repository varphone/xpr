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
#  define XPR_AlignedUpTo(v, x)  (((v)+((x)-1)) & (~((x)-1))) ///< @brief 将数值 v 向上对齐到 x
#endif

#ifndef XPR_MakeFourCC
/// @brief 合成 FourCC 编码
#  define XPR_MakeFourCC( a, b, c, d ) \
     ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
     | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

#ifndef XPR_MakeVersion
#  define XPR_MakeVersion(a, b, c) (a<<24|b<<16|c) ///< 生成 32 位版本号
#endif

#ifndef _countof
#  define _countof(_array) (sizeof(_array) / sizeof(_array[0])) ///< 计算数组大小
#endif

#ifndef offsetof
#  define offsetof(st, m) __builtin_offsetof(st, m) ///< 计算结构体成员偏移
#endif

#ifndef XPR_FALSE
#  define XPR_FALSE 0 ///< 布尔假值
#endif

#ifndef XPR_TRUE
#  define XPR_TRUE 1 ///< 布尔真值
#endif

// FourCC
//==============================================================================
/// @addtogroup xprcommon-avfourcc AV FourCC
/// @{
///

#ifndef AV_FOURCC_NULL
#  define AV_FOURCC_NULL XPR_MakeFourCC('N', 'U', 'L', 'L') ///< NULL
#endif

#ifndef AV_FOURCC_G711A
#  define AV_FOURCC_G711A XPR_MakeFourCC('7', '1', '1', 'A') ///< G711.A audio codec
#endif

#ifndef AV_FOURCC_G711U
#  define AV_FOURCC_G711U XPR_MakeFourCC('7', '1', '1', 'U') ///< G711.U audio codec
#endif

#ifndef AV_FOURCC_JPEG
#  define AV_FOURCC_JPEG  XPR_MakeFourCC('J', 'P', 'E', 'G') ///< JPEG/MJPEG video codec
#endif

#ifndef AV_FOURCC_H264
#  define AV_FOURCC_H264  XPR_MakeFourCC('a', 'v', 'c', '1') ///< H264 video codec
#endif

#ifndef AV_FOURCC_HKMI
#  define AV_FOURCC_HKMI  XPR_MakeFourCC('H', 'K', 'M', 'I') ///< Hikvision media information data codec
#endif

#ifndef AV_FOURCC_MP2P
#  define AV_FOURCC_MP2P  XPR_MakeFourCC('M', 'P', '2', 'P') ///< MPEG2-PS data codec
#endif

#ifndef AV_FOURCC_MP2T
#  define AV_FOURCC_MP2T  XPR_MakeFourCC('M', 'P', '2', 'T') ///< MPEG-TS data codec
#endif

#ifndef AV_FOURCC_PCMA
#  define AV_FOURCC_PCMA XPR_MakeFourCC('P', 'C', 'M', 'A') ///< PCM-alaw audio codec
#endif

#ifndef AV_FOURCC_PCMU
#  define AV_FOURCC_PCMU XPR_MakeFourCC('P', 'C', 'M', 'U') ///< PCM-ulaw audio codec
#endif

#ifndef AV_FOURCC_S16B
#  define AV_FOURCC_S16B XPR_MakeFourCC('S', '1', '6', 'B') ///< Signed 16 bit PCM big-endian audio codec
#endif

#ifndef AV_FOURCC_S16L
#  define AV_FOURCC_S16L XPR_MakeFourCC('S', '1', '6', 'L') ///< Signed 16 bit PCM little-endian audio codec
#endif

#ifndef AV_FOURCC_MDSD
#  define AV_FOURCC_MDSD XPR_MakeFourCC('M', 'D', 'S', 'D') ///< Motion detection summy data codec
#endif

#ifndef AV_FOURCC_XDMI
#  define AV_FOURCC_XDMI XPR_MakeFourCC('X', 'D', 'M', 'I') ///< Xian Dian media information data codec
#endif

#ifndef AV_FOURCC_XPMI
#  define AV_FOURCC_XPMI XPR_MakeFourCC('X', 'P', 'M', 'I') ///< X Portable media information data codec
#endif

/// @}
///

/// @}
///

#endif // XPR_COMMON_H

