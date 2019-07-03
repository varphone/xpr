/*
 * File: xpr_common.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 通用定义，定义通用的宏，数据类型
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:26:22 Wednesday, 3 July
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
#ifndef XPR_COMMON_H
#define XPR_COMMON_H

#if !defined(XPR_STATIC)
#  if defined(XPR_EXPORTS)
#    if defined(_MSC_VER)
#      define XPR_API __declspec(dllexport)
#    else
#      define XPR_API
#    endif // defined(_MSC_VER)
#  else
#    if defined(_MSC_VER)
#      define XPR_API __declspec(dllimport)
#    else
#      define XPR_API
#    endif // defined(_MSC_VER)
#  endif // defined(XPR_EXPORTS)
#else
#define XPR_API
#endif // !defined(XPR_STATIC)

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

#ifndef AV_FOURCC_H265
///
/// H265 video codec
///
#  define AV_FOURCC_H265  XPR_MakeFourCC('h', 'e', 'v', 'c')
#endif

#ifndef AV_FOURCC_HEVC
///
/// H265 video codec
///
#  define AV_FOURCC_HEVC  XPR_MakeFourCC('h', 'e', 'v', 'c')
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

#ifndef AV_FOURCC_BMPI
///
/// BMP image data codec
///
#  define AV_FOURCC_BMPI XPR_MakeFourCC('B', 'M', 'P', 'I')
#endif

#ifndef AV_FOURCC_JPGI
///
/// JPEG image data codec
///
#  define AV_FOURCC_JPGI XPR_MakeFourCC('J', 'P', 'G', 'I')
#endif

#ifndef AV_FOURCC_PNGI
///
/// PNG image data codec
///
#  define AV_FOURCC_PNGI XPR_MakeFourCC('P', 'N', 'G', 'I')
#endif

#endif // XPR_COMMON_H
