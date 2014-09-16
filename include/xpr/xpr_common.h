#ifndef XPR_COMMON_H
#define XPR_COMMON_H

/// 将数值 v 向上对齐到 x
#ifndef XPR_AlignedUpTo
#  define XPR_AlignedUpTo(v, x)  (((v)+((x)-1)) & (~((x)-1)))
#endif

#ifndef XPR_MakeFourCC
#  define XPR_MakeFourCC( a, b, c, d ) \
     ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
     | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

#ifndef XPR_MakeVersion
#  define XPR_MakeVersion(a, b, c) (a<<24|b<<16|c) ///< 生成 32 位版本号
#endif

#ifndef _countof
#  define _countof(_array) (sizeof(_array) / sizeof(_array[0])) 
#endif

#ifndef offsetof
#  define offsetof(st, m) __builtin_offsetof(st, m)
#endif

// FourCC
//==============================================================================
#ifndef AV_FOURCC_NULL
#  define AV_FOURCC_NULL XPR_MakeFourCC('N', 'U', 'L', 'L')
#endif

#ifndef AV_FOURCC_G711A
#  define AV_FOURCC_G711A XPR_MakeFourCC('7', '1', '1', 'A')
#endif

#ifndef AV_FOURCC_G711U
#  define AV_FOURCC_G711U XPR_MakeFourCC('7', '1', '1', 'U')
#endif

#ifndef AV_FOURCC_JPEG
#  define AV_FOURCC_JPEG  XPR_MakeFourCC('J', 'P', 'E', 'G')
#endif

#ifndef AV_FOURCC_H264
#  define AV_FOURCC_H264  XPR_MakeFourCC('a', 'v', 'c', '1')
#endif

#ifndef AV_FOURCC_HKMI
#  define AV_FOURCC_HKMI  XPR_MakeFourCC('H', 'K', 'M', 'I')
#endif

#ifndef AV_FOURCC_MP2P
#  define AV_FOURCC_MP2P  XPR_MakeFourCC('M', 'P', '2', 'P')
#endif

#ifndef AV_FOURCC_MP2T
#  define AV_FOURCC_MP2T  XPR_MakeFourCC('M', 'P', '2', 'T')
#endif

#ifndef AV_FOURCC_PCMA
#  define AV_FOURCC_PCMA XPR_MakeFourCC('P', 'C', 'M', 'A')
#endif

#ifndef AV_FOURCC_PCMU
#  define AV_FOURCC_PCMU XPR_MakeFourCC('P', 'C', 'M', 'U')
#endif

#ifndef AV_FOURCC_S16B
#  define AV_FOURCC_S16B XPR_MakeFourCC('S', '1', '6', 'B')
#endif

#ifndef AV_FOURCC_S16L
#  define AV_FOURCC_S16L XPR_MakeFourCC('S', '1', '6', 'L')
#endif

#ifndef AV_FOURCC_MDSD
#  define AV_FOURCC_MDSD XPR_MakeFourCC('M', 'D', 'S', 'D')
#endif

#ifndef AV_FOURCC_XDMI
#  define AV_FOURCC_XDMI  XD_MakeFourCC('X', 'D', 'M', 'I')
#endif

#ifndef AV_FOURCC_XPMI
#  define AV_FOURCC_XPMI  XD_MakeFourCC('X', 'P', 'M', 'I')
#endif

#endif // XPR_COMMON_H

