/*
 * File: xpr_pes.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * MPEG-PS 打包接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:50:58 Wednesday, 3 July
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
#ifndef XPR_PES_H
#define XPR_PES_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_PES_HDR_SIZE 40

#ifndef XPR_PES_TYPE_DEFINED
#define XPR_PES_TYPE_DEFINED
struct XPR_PES;
typedef struct XPR_PES XPR_PES;
#endif // XPR_PES_TYPE_DEFINED

#ifndef XPR_PES_HEADER_TYPE_DEFINED
#define XPR_PES_HEADER_TYPE_DEFINED
struct XPR_PES_Header;
typedef struct XPR_PES_Header XPR_PES_Header;
#endif // XPR_PES_HEADER_TYPE_DEFINED

#ifndef XPR_PES_WRITECALLBACK_TYPE_DEFINED
#define XPR_PES_WRITECALLBACK_TYPE_DEFINED
typedef int (* XPR_PES_WriteCallback)(const uint8_t* data, int length,
                                      void* opaque);
#endif // XPR_PES_WRITECALLBACK_TYPE_DEFINED

XPR_API XPR_PES* XPR_PES_Open(const char* url);

XPR_API int XPR_PES_Close(XPR_PES* p);

XPR_API int XPR_PES_AddWriteCallback(XPR_PES* f, XPR_PES_WriteCallback cb,
                                     void* opaque);

XPR_API int XPR_PES_WriteHeader(XPR_PES* p, const XPR_PES_Header* hdr);

XPR_API int XPR_PES_WriteFrame(XPR_PES* p, const uint8_t* data, int length,
                               int codec, int64_t pts);

XPR_API int XPR_PES_WriteFramePartial(XPR_PES* p, const uint8_t* data,
                                      int length, int codec, int64_t pts,
                                      int firstPart);

XPR_API int XPR_PES_WriteTailer(XPR_PES* p);

XPR_API int XPR_PES_WriteOpaque(XPR_PES* p, const uint8_t* data, int length);

XPR_API XPR_PES_Header* XPR_PES_HeaderNew(XPR_PES* p);

XPR_API void XPR_PES_HeaderInit(XPR_PES* p, XPR_PES_Header* hdr);

XPR_API void XPR_PES_HeaderDestroy(XPR_PES_Header* hdr);

XPR_API void XPR_PES_HeaderSetAudioPID(XPR_PES_Header* hdr, int flags);

XPR_API void XPR_PES_HeaderSetVideoPID(XPR_PES_Header* hdr, int flags);

#ifdef __cplusplus
}
#endif

#endif // XPR_PES_H
