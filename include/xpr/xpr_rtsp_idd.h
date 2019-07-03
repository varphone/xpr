﻿ /*
 * File: xpr_rtsp_idd.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 通用 RTSP 交错数据分离器
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:25:25 Friday, 25 November
 * Last Modified : 2019-07-03 04:46:02 Wednesday, 3 July
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
#ifndef XPR_RTSP_IDD_H
#define XPR_RTSP_IDD_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_RTSP_IDD_MAX_BUFFER_SIZE    8196

#ifndef XPR_RTSP_IDD_TYPE_DEFINED
#define XPR_RTSP_IDD_TYPE_DEFINED
struct XPR_RTSP_IDD;
typedef struct XPR_RTSP_IDD XPR_RTSP_IDD;
#endif // XPR_RTSP_IDD_TYPE_DEFINED

#ifndef XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED
#define XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED
///
/// RTSP 已分离数据类型
///
typedef enum XPR_RTSP_IDD_DataType {
    XPR_RTSP_IDD_DATA_TYPE_BIN,     ///< 二进制数据, 例如: rtp, rtcp ..
    XPR_RTSP_IDD_DATA_TYPE_HDR,     ///< RTSP 请求或响应头
    XPR_RTSP_IDD_DATA_TYPE_SDP,     ///< SDP 数据
} XPR_RTSP_IDD_DataType;
#endif //XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED

///
/// RTSP 交错数据分离器已分离数据处理函数
///
/// @param [in] opaque      用户关联数据
/// @param [in] channel     数据所属通道
/// @param [in] dataType    数据类型
/// @param [in] data        已分离数据
/// @param [in] length      已分离数据字节数
/// @retval XPR_ERR_OK      处理成功
/// @retval XPR_ERR_ERROR   处理失败
typedef int (*XPR_RTSP_IDD_DataHandler)(void* opaque, int channel, int dataType,
                                        uint8_t* data, int length);

#ifndef XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED
#define XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED
///
/// RTSP 交错数据分离器参数类型
///
typedef enum XPR_RTSP_IDD_ParamType {
    XPR_RTSP_IDD_PARAM_DATA_HANDLER,    ///< 数据处理回调函数
    XPR_RTSP_IDD_PARAM_OPAQUE,          ///< 用户关联数据
} XPR_RTSP_IDD_ParamType;
#endif // XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED

///
/// 创建一个 RTSP 交错数据分离器
///
/// @retval NULL    创建失败
/// @retval Other   已创建好的数据分离器句柄
XPR_API XPR_RTSP_IDD* XPR_RTSP_IDD_New(void);

///
/// 销毁一个已创建的 RTSP 交错数据分离器
///
XPR_API int XPR_RTSP_IDD_Destroy(XPR_RTSP_IDD* idd);

XPR_API int XPR_RTSP_IDD_PushData(XPR_RTSP_IDD* idd, const uint8_t* data,
                                  int length);

XPR_API int XPR_RTSP_IDD_SetParam(XPR_RTSP_IDD* idd, int param,
                                  const void* data, int length);

XPR_API int XPR_RTSP_IDD_GetParam(XPR_RTSP_IDD* idd, int param, void* buffer,
                                  int* size);

#ifdef __cplusplus
}
#endif

#endif // XPR_RTSP_IDD_H
