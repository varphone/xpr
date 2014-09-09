#ifndef XPR_COMMON_H
#define XPR_COMMON_H

#ifndef XPR_MakeFourCC
#define XPR_MakeFourCC( a, b, c, d ) \
    ( ((uint32_t)a) | ( ((uint32_t)b) << 8 ) \
    | ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

#ifndef XPR_MakeVersion
#define XPR_MakeVersion(a, b, c) (a<<24|b<<16|c) ///< 生成 32 位版本号
#endif

#ifndef _countof
#define _countof(_array) (sizeof(_array) / sizeof(_array[0])) 
#endif

#endif // XPR_COMMON_H

