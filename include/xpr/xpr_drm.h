﻿/*
 * File: xpr_drm.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 数字版权管理
 * 
 * 提供各种数字版权管理接口, 例如: 硬件验证, 获取序列号等等
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:19:52 Wednesday, 3 July
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
#ifndef XPR_DRM_H
#define XPR_DRM_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// 序列号位宽
///
#define XPR_DRM_SERIAL_BTIS  64

///
/// 序列号字节数
///
#define XPR_DRM_SERIAL_SIZE  8

#define XPR_DRM_SERIAL_STRING_NULL  "0000000000000000"
#define XPR_DRM_UUID_STRING_NULL    "00000000-0000-0000-0000-000000000000"

#ifndef XPR_DRM_CONFIGTYPE_TYPE_DEFINED
#define XPR_DRM_CONFIGTYPE_TYPE_DEFINED
///
/// XPR_DRM 配置参数类型
///
enum XPR_DRM_ConfigType {
    XPR_DRM_CFG_I2C_DEV,        ///< I2C 设备, 数据类型: const char*
    XPR_DRM_CFG_I2C_SLAVE_ADDR, ///< I2C 设备从地址, 数据类型: int
};
#endif // XPR_DRM_CONFIGTYPE_TYPE_DEFINED

///
/// 配置 XPR_DRM 模块
///
/// @param [in] cfg     配置参数编号
/// @param [in] data    配置参数数据
/// @param [in] length  配置参数长度
/// @retval XPR_ERR_SUCCESS 配置成功
/// @retval XPR_ERR_ERROR 配置失败
/// @note 此接口需要在 XPR_DRM_Init() 之前调用
XPR_API int XPR_DRM_Config(int cfg, const void* data, int length);

///
/// 初始化 XPR_DRM 模块
///
/// @retval XPR_ERR_SUCCESS 初始化成功
/// @retval XPR_ERR_ERROR 初始化失败
XPR_API int XPR_DRM_Init(void);

///
/// 释放 XPR_DRM 模块已分配的资源
///
/// @retval XPR_ERR_SUCCESS 成功
/// @retval XPR_ERR_ERROR 失败
XPR_API int XPR_DRM_Fini(void);

///
/// 验证 XPR_DRM 数据
///
/// @retval XPR_ERR_SUCCESS 验证成功
/// @retval XPR_ERR_ERROR 验证失败
XPR_API int XPR_DRM_Verify(void);

///
/// 获取 XPR_DRM 序列号
///
/// @param [in,out] buffer  用于接受序列号数据的缓冲区
/// @param [in,out] size  用于接受序列号数据的缓冲区长度, 并返回实际数据长度
/// @retval XPR_ERR_SUCCESS 获取成功
/// @retval XPR_ERR_ERROR 获取失败
XPR_API int XPR_DRM_GetSerial(uint8_t* buffer, int* size);

///
/// 获取 XPR_DRM 序列号字符串
///
/// @retval NULL 获取失败
/// @retval Other 字符串格式的序列号
XPR_API const char* XPR_DRM_GetSerialString(void);

///
/// 获取 XPR_DRM 全球唯一序列号字符串
///
/// @retval NULL 获取失败
/// @retval Other 字符串格式的全球唯一序列号
XPR_API const char* XPR_DRM_GetUuidString(void);

///
/// 安装 XPR_DRM 序列号
///
/// @param [in,out] data    用于安装的序列号数据
/// @param [in,out] length  用于安装的序列号数据的长度
/// @retval XPR_ERR_SUCCESS 安装成功
/// @retval XPR_ERR_ERROR 安装失败
XPR_API int XPR_DRM_InstallSerial(const uint8_t* data, int length);

#ifdef __cplusplus
}
#endif

#endif // XPR_DRM_H
