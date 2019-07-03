/*
 * File: xpr_arr.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 音频重采样操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:32:16 Wednesday, 3 July
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
#ifndef XPR_ARR_H
#define XPR_ARR_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ARR_TYPE_DEFINED
#define XPR_ARR_TYPE_DEFINED
struct XPR_ARR;
typedef struct XPR_ARR XPR_ARR;
#endif // XPR_ARR_TYPE_DEFINED

XPR_API int XPR_ARR_Init(void);

XPR_API void XPR_ARR_Fini(void);

XPR_API XPR_ARR* XPR_ARR_Open(void);

XPR_API int XPR_ARR_Close(XPR_ARR* r);

XPR_API void XPR_ARR_SetBitsPerSample(XPR_ARR* r, int bps);

XPR_API void XPR_ARR_SetSampleRates(XPR_ARR* r, int from, int to);

XPR_API void XPR_ARR_SetChannels(XPR_ARR* r, int channels);

XPR_API int XPR_ARR_GetInputSamples(XPR_ARR* r);

XPR_API int XPR_ARR_GetOutputSamples(XPR_ARR* r);

XPR_API int XPR_ARR_Transform(XPR_ARR* r, void* src, void* dst);

#ifdef __cplusplus
}
#endif

#endif // XPR_ARR_H
